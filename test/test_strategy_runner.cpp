#include <gtest/gtest.h>

#include "Exchange.h"
#include "OrderFactory.h"
#include "PreTradeRisk.h"
#include "StrategyRunner.h"
#include "ThresholdStrategy.h"
#include "Tick.h"

/// 测试 StrategyRunner 的 on_tick
/// 方法在不同情况下的行为，确保策略、风控、订单工厂与交易所按正确顺序协作。


/// 测试策略未产生交易意图时，订单工厂不消耗 order id。
TEST(StrategyRunnerTest, NoSignalDoesNotConsumeOrderId) {
    // quantity 固定为 10：
    // price=150 位于阈值区间内，不产生交易意图
    // price=50 低于买入阈值，产生合法 Buy intent
    ThresholdStrategy strategy(100.0, 200.0, 10);
    PreTradeRisk risk(10, 1500.0);
    OrderFactory order_factory(1000);
    Exchange exchange;
    StrategyRunner runner(strategy, risk, order_factory, exchange);

    // 1. 策略未产生交易意图，返回 NoSignal，订单工厂不消耗 order id
    RunResult no_signal = runner.on_tick(Tick(1000, 150.0, 1));

    EXPECT_EQ(no_signal.outcome, RunOutcome::NoSignal);
    EXPECT_FALSE(no_signal.risk_decision.has_value());
    EXPECT_TRUE(no_signal.fills.empty());
    EXPECT_EQ(exchange.get_order_manager().get_order_size(), 0u);

    // 2. 策略产生交易意图，风控通过，返回 Submitted，订单工厂消耗 order id
    RunResult submitted = runner.on_tick(Tick(1001, 50.0, 1));

    EXPECT_EQ(submitted.outcome, RunOutcome::Submitted);
    EXPECT_FALSE(submitted.risk_decision.has_value());
    EXPECT_TRUE(submitted.fills.empty());

    auto order = exchange.get_order_manager().get_order_content(1000);
    ASSERT_TRUE(order.has_value());
    EXPECT_EQ(order->get_id(), 1000u);
}

/// 测试风控拒绝意图时，订单工厂不消耗 order id。
TEST(StrategyRunnerTest, RiskRejectedDoesNotConsumeOrderId) {
    // quantity 固定为 10：
    // price=250 → notional=2500，超过 1500，被风控拒绝
    // price=50  → notional=500，通过风控
    ThresholdStrategy strategy(100.0, 200.0, 10);
    PreTradeRisk risk(10, 1500.0);
    OrderFactory order_factory(1000);
    Exchange exchange;
    StrategyRunner runner(strategy, risk, order_factory, exchange);

    // 1. 风控拒绝意图，返回 RiskRejected，订单工厂不消耗 order id
    RunResult rejected = runner.on_tick(Tick(1000, 250.0, 1));

    EXPECT_EQ(rejected.outcome, RunOutcome::RiskRejected);
    ASSERT_TRUE(rejected.risk_decision.has_value());
    EXPECT_EQ(*rejected.risk_decision, RiskDecision::NotionalLimitExceeded);
    EXPECT_TRUE(rejected.fills.empty());
    EXPECT_EQ(exchange.get_order_manager().get_order_size(), 0u);

    // 2. 风控通过意图，返回 Submitted，订单工厂消耗 order id
    RunResult submitted = runner.on_tick(Tick(1001, 50.0, 1));

    EXPECT_EQ(submitted.outcome, RunOutcome::Submitted);
    EXPECT_FALSE(submitted.risk_decision.has_value());
    EXPECT_TRUE(submitted.fills.empty());

    auto order = exchange.get_order_manager().get_order_content(1000);
    ASSERT_TRUE(order.has_value());
    EXPECT_EQ(order->get_id(), 1000u);
}
