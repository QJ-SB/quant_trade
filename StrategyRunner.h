#pragma once

#include <optional>
#include <vector>

#include "Exchange.h"
#include "Fill.h"
#include "OrderFactory.h"
#include "PreTradeRisk.h"
#include "ThresholdStrategy.h"
#include "Tick.h"

/// 描述一次 on_tick 调用的结果状态。
enum class RunOutcome {
    NoSignal,          // 策略未产生交易意图
    RiskRejected,      // 风控拒绝，未创建订单
    ExchangeRejected,  // 交易所拒绝（如重复 id）
    Submitted,         // 订单已提交至交易所（无论是否成交）
};

/// 聚合一次 on_tick 调用的完整结果。
struct RunResult {
    RunOutcome outcome;
    std::optional<RiskDecision>
        risk_decision;  // RiskRejected 时有值，其他情况为 std::nullopt
    std::vector<Fill> fills;
};

/// 将策略、风控、订单工厂与交易所按正确顺序串联。
///
/// 调用方必须保证所引用的对象生命周期覆盖 StrategyRunner 的使用区间。
class StrategyRunner {
public:
    /// 构造执行器并持有外部组件的引用。
    /// @param strategy 策略实例（只读）
    /// @param risk 风控实例（只读）
    /// @param order_factory 订单工厂（非 const，会分配递增 id）
    /// @param exchange 交易所（非 const，会登记并撮合订单）
    StrategyRunner(const ThresholdStrategy& strategy, const PreTradeRisk& risk,
                   OrderFactory& order_factory, Exchange& exchange);

    /// 接收一笔 Tick，依次执行 策略→风控→订单创建→交易所提交 的完整链路。
    /// @param tick 行情快照
    /// @return RunResult 包含结果状态、风控决策（若被拒）与成交记录
    [[nodiscard]] RunResult on_tick(const Tick& tick);

private:
    const ThresholdStrategy& m_strategy;
    const PreTradeRisk& m_risk;
    OrderFactory& m_order_factory;
    Exchange& m_exchange;
};
