#include <gtest/gtest.h>

#include "StrategyRunner.h"


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

/// 测试交易所成功提交、并发生交易后，StrategyRunner 能正确返回成交信息 fills
TEST(StrategyRunnerTest, SubmittedOrderReturnsExchangeFills) {
    ThresholdStrategy strategy(100.0, 200.0, 10);
    PreTradeRisk risk(10, 1500.0);
    OrderFactory order_factory(1000);
    Exchange exchange;
    StrategyRunner runner(strategy, risk, order_factory, exchange);

    // 先放入一笔卖单作为 maker：
    // 后续策略生成 50.0 的 Buy taker，可以吃掉 49.0 的 Sell maker。
    auto maker_submission =
        exchange.submit_order(Order(9000, OrderDirection::Sell, 49.0, 10));

    ASSERT_TRUE(maker_submission.has_value());
    EXPECT_TRUE(maker_submission->empty());

    // 策略生成 Buy 意图，风控通过，提交交易所后产生成交记录
    RunResult result = runner.on_tick(Tick(1000, 50.0, 1));

    EXPECT_EQ(result.outcome, RunOutcome::Submitted);
    EXPECT_FALSE(result.risk_decision.has_value());

    ASSERT_EQ(result.fills.size(), 1u);

    const Fill& fill = result.fills.front();
    EXPECT_EQ(fill.taker_id, 1000u);
    EXPECT_EQ(fill.maker_id, 9000u);
    EXPECT_EQ(fill.quantity, 10);
    EXPECT_DOUBLE_EQ(fill.price, 49.0);
}

/// 测试交易所拒绝重复 id 时，StrategyRunner 能正确返回 ExchangeRejected,
/// 并且不影响 Exchange 中原有订单
TEST(StrategyRunnerTest, DuplicateIdMapsToExchangeRejected) {
    ThresholdStrategy strategy(100.0, 200.0, 10);
    PreTradeRisk risk(10, 1500.0);
    OrderFactory order_factory(1000);
    Exchange exchange;

    // 预先占用 id=1000；订单不成交，保持在 Exchange 中。
    auto existing_submission =
        exchange.submit_order(Order(1000, OrderDirection::Sell, 300.0, 10));

    ASSERT_TRUE(existing_submission.has_value());
    EXPECT_TRUE(existing_submission->empty());

    StrategyRunner runner(strategy, risk, order_factory, exchange);

    // price=50 产生合法 Buy intent；OrderFactory 创建 id=1000，
    // 但 Exchange 中该 id 已存在，因此提交被拒绝。
    RunResult result = runner.on_tick(Tick(1000, 50.0, 1));

    EXPECT_EQ(result.outcome, RunOutcome::ExchangeRejected);
    EXPECT_FALSE(result.risk_decision.has_value());
    EXPECT_TRUE(result.fills.empty());

    // 重复提交不得覆盖或删除 Exchange 中原有订单。
    EXPECT_EQ(exchange.get_order_manager().get_order_size(), 1u);

    auto existing_order = exchange.get_order_manager().get_order_content(1000);

    ASSERT_TRUE(existing_order.has_value());
    EXPECT_EQ(existing_order->get_direction(), OrderDirection::Sell);
    EXPECT_DOUBLE_EQ(existing_order->get_price(), 300.0);
    EXPECT_EQ(existing_order->get_quantity(), 10);
    EXPECT_EQ(existing_order->get_status(), OrderStatus::PENDING);
}
