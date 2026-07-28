#include "StrategyRunner.h"

#include <utility>  // std::move()

#include "Order.h"

/// 构造执行器，按成员声明顺序用外部组件引用初始化。
/// @param strategy 策略实例（只读），决定何时产生交易意图
/// @param risk 风控实例（只读），拦截非法/超限意图
/// @param order_factory 订单工厂（非 const），分配递增 order id
/// @param exchange 交易所（非 const），登记并撮合订单
StrategyRunner::StrategyRunner(const ThresholdStrategy& strategy,
                               const PreTradeRisk& risk,
                               OrderFactory& order_factory, Exchange& exchange)
    : m_strategy(strategy), m_risk(risk), m_order_factory(order_factory),
      m_exchange(exchange) {}

/// 接收一笔 Tick，按 Strategy → Risk → OrderFactory → Exchange 的固定顺序
/// 执行完整交易链路。
///
/// @param tick 行情快照，透传给策略判断
/// @return RunResult 聚合本轮结果：outcome 标识到达的终点，
///         risk_decision 仅在 RiskRejected 时有值，
///         fills 仅在 Submitted 时可能非空
RunResult StrategyRunner::on_tick(const Tick& tick) {
    // 1. 策略判断是否产生意图
    auto intent = m_strategy.on_tick(tick);
    if (!intent.has_value()) {
        return {RunOutcome::NoSignal, std::nullopt, {}};
    }

    // 2. 风控校验：被拒则直接返回，不创建订单
    RiskDecision decision = m_risk.check(*intent);
    if (decision != RiskDecision::Accepted) {
        return {RunOutcome::RiskRejected, decision, {}};
    }

    // 3. 创建正式订单（消耗递增 order id）
    Order order = m_order_factory.create_order(*intent);

    // 4. 提交交易所
    auto submission = m_exchange.submit_order(order);
    if (!submission.has_value()) {
        return {RunOutcome::ExchangeRejected, std::nullopt, {}};
    }

    // std::move: 将 submission 内的 vector<Fill> 资源转移给 RunResult，
    // 避免复制整组成交记录
    return {
        RunOutcome::Submitted,
        std::nullopt,
        std::move(*submission),
    };
}
