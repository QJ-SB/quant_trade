#include <gtest/gtest.h>
#include <utility>
#include <vector>

#include "Exchange.h"
#include "Feed.h"
#include "Order.h"
#include "OrderFactory.h"
#include "PreTradeRisk.h"
#include "StrategyRunner.h"
#include "ThresholdStrategy.h"
#include "Tick.h"

/// 功能描述：验证完整执行链路 Feed→策略→订单工厂→交易所→OMS 的全流程集成
TEST(FeedStrategyRunnerIntegrationTest,
     FeedDrivesRunnerThroughNoSignalRiskRejectAndSubmission) {
    ThresholdStrategy strategy(100.0, 200.0, 10);
    PreTradeRisk risk(10, 1500.0);
    OrderFactory order_factory(1000);
    Exchange exchange;
    StrategyRunner runner(strategy, risk, order_factory, exchange);

    // 为最后一笔 Buy intent 预先放入可成交的 Sell maker。
    auto maker_submission =
        exchange.submit_order(Order(9000, OrderDirection::Sell, 49.0, 10));

    ASSERT_TRUE(maker_submission.has_value());
    ASSERT_TRUE(maker_submission->empty());

    std::vector<Tick> ticks = {
        {1000, 150.0, 1},  // NoSignal
        {1001, 250.0, 1},  // Sell intent，notional 超限
        {1002, 50.0, 1},   // Buy intent，通过并与 maker 成交
    };


    // 定义外部变量存储回调结果result（lambda 内不堆积断言，统一后置校验）
    std::vector<RunResult> results;

    // Feed 回调链路：tick→on_tick→生成意向→创建订单→提交订单→保存结果
    Feed feed(std::move(ticks), [&runner, &results](const Tick& tick) {
        results.push_back(runner.on_tick(tick));
    });

    feed.run();

    ASSERT_EQ(results.size(), 3u);

    EXPECT_EQ(results[0].outcome, RunOutcome::NoSignal);
    EXPECT_FALSE(results[0].risk_decision.has_value());
    EXPECT_TRUE(results[0].fills.empty());

    EXPECT_EQ(results[1].outcome, RunOutcome::RiskRejected);
    ASSERT_TRUE(results[1].risk_decision.has_value());
    EXPECT_EQ(*results[1].risk_decision, RiskDecision::NotionalLimitExceeded);
    EXPECT_TRUE(results[1].fills.empty());

    EXPECT_EQ(results[2].outcome, RunOutcome::Submitted);
    EXPECT_FALSE(results[2].risk_decision.has_value());
    ASSERT_EQ(results[2].fills.size(), 1u);

    const Fill& fill = results[2].fills.front();
    EXPECT_EQ(fill.taker_id, 1000u);
    EXPECT_EQ(fill.maker_id, 9000u);
    EXPECT_EQ(fill.quantity, 10);
    EXPECT_DOUBLE_EQ(fill.price, 49.0);
}
