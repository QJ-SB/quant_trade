#include <cstdint>  // std::uint64_t
#include <iostream>
#include <utility>  // std::move
#include <vector>

#include "Exchange.h"
#include "Feed.h"
#include "OrderFactory.h"
#include "PreTradeRisk.h"
#include "StrategyRunner.h"
#include "ThresholdStrategy.h"
#include "Tick.h"

namespace {

// 把 RunOutcome 枚举转成可读字符串，仅用于控制台输出
const char* run_outcome_name(RunOutcome outcome) {
    switch (outcome) {
    case RunOutcome::NoSignal:
        return "NoSignal";
    case RunOutcome::RiskRejected:
        return "RiskRejected";
    case RunOutcome::ExchangeRejected:
        return "ExchangeRejected";
    case RunOutcome::Submitted:
        return "Submitted";
    }

    return "Unknown";
}

// 把 RiskDecision 枚举转成可读字符串，仅用于控制台输出
const char* risk_decision_name(RiskDecision decision) {
    switch (decision) {
    case RiskDecision::Accepted:
        return "Accepted";
    case RiskDecision::InvalidPrice:
        return "InvalidPrice";
    case RiskDecision::NonPositiveQuantity:
        return "NonPositiveQuantity";
    case RiskDecision::QuantityLimitExceeded:
        return "QuantityLimitExceeded";
    case RiskDecision::NotionalLimitExceeded:
        return "NotionalLimitExceeded";
    }

    return "Unknown";
}

// 打印单次 tick 的处理结果；fills 可能为空（比如风控拒绝或没有对手盘）
void print_run_result(const Tick& tick, const RunResult& result) {
    std::cout << "timestamp=" << tick.get_timestamp()
              << " price=" << tick.get_price()
              << " volume=" << tick.get_volume()
              << " outcome=" << run_outcome_name(result.outcome)
              << " fills=" << result.fills.size();

    // risk_decision 只在风控拒绝时有值，正常通过时不显示
    if (result.risk_decision.has_value()) {
        std::cout << " risk=" << risk_decision_name(*result.risk_decision);
    }

    std::cout << '\n';

    // 逐笔打印成交明细；没有成交时这段循环不执行
    for (const auto& fill : result.fills) {
        std::cout << "  fill:"
                  << " taker_id=" << fill.taker_id
                  << " maker_id=" << fill.maker_id << " price=" << fill.price
                  << " quantity=" << fill.quantity << '\n';
    }
}

}  // namespace


int main() {
    // ---- 策略参数 ----
    constexpr double kBuyBelow = 100.0;  // 价格低于此值时产生买入信号
    constexpr double kSellAbove = 200.0;  // 价格高于此值时产生卖出信号
    constexpr int kOrderQuantity = 10;  // 每次下单数量

    // ---- 风控参数 ----
    constexpr int kMaxOrderQuantity = 10;          // 单笔最大数量
    constexpr double kMaxOrderNotional = 1500.0;   // 单笔最大名义金额
    constexpr std::uint64_t kFirstOrderId = 1000;  // 订单 ID 起始值

    // ---- 内存数据源：三个 tick 分别验证 NoSignal / RiskRejected / Submitted
    std::vector<Tick> ticks = {
        {1000, 150.0, 10},  // 150 在 100~200 之间 → NoSignal

        {1001, 250.0, 10},  // 250 > 200，卖出意图
                            // 但名义金额 2500 > 1500 → RiskRejected

        {1002, 50.0, 10},  // 50 < 100，买入意图，名义金额 500 < 1500 →
                           // Submitted，但无对手盘
    };

    // ---- 组装 composition root：Feed → StrategyRunner → Exchange ----
    ThresholdStrategy strategy(kBuyBelow, kSellAbove, kOrderQuantity);
    PreTradeRisk risk(kMaxOrderQuantity, kMaxOrderNotional);
    OrderFactory order_factory(kFirstOrderId);
    Exchange exchange;

    StrategyRunner runner(strategy, risk, order_factory, exchange);

    // Feed 持有 ticks，逐个回调处理；std::move 避免拷贝
    Feed feed(std::move(ticks), [&runner](const Tick& tick) {
        const RunResult result = runner.on_tick(tick);
        print_run_result(tick, result);  // 打印结果到控制台
    });

    feed.run();  // 逐笔调用 handler，完成整个链路的处理

    return 0;
}
