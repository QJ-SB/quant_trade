#include <gtest/gtest.h>

#include "Exchange.h"
#include "OrderFactory.h"
#include "PreTradeRisk.h"
#include "StrategyRunner.h"
#include "ThresholdStrategy.h"
#include "Tick.h"

TEST(StrategyRunnerTest, RiskRejectedDoesNotConsumeOrderId) {
    // quantity 固定为 10：
    // price=250 → notional=2500，超过 1500，被风控拒绝
    // price=50  → notional=500，通过风控
    ThresholdStrategy strategy(100.0, 200.0, 10);
    PreTradeRisk risk(10, 1500.0);
    OrderFactory order_factory(1000);
    Exchange exchange;
    StrategyRunner runner(strategy, risk, order_factory, exchange);

    RunResult rejected = runner.on_tick(Tick(1000, 250.0, 1));

    EXPECT_EQ(rejected.outcome, RunOutcome::RiskRejected);
    ASSERT_TRUE(rejected.risk_decision.has_value());
    EXPECT_EQ(*rejected.risk_decision, RiskDecision::NotionalLimitExceeded);
    EXPECT_TRUE(rejected.fills.empty());
    EXPECT_EQ(exchange.get_order_manager().get_order_size(), 0u);

    RunResult submitted = runner.on_tick(Tick(1001, 50.0, 1));

    EXPECT_EQ(submitted.outcome, RunOutcome::Submitted);
    EXPECT_FALSE(submitted.risk_decision.has_value());
    EXPECT_TRUE(submitted.fills.empty());

    auto order = exchange.get_order_manager().get_order_content(1000);
    ASSERT_TRUE(order.has_value());
    EXPECT_EQ(order->get_id(), 1000u);
}
