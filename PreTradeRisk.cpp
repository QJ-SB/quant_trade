#include "PreTradeRisk.h"

#include <cmath>  // std::isfinite

PreTradeRisk::PreTradeRisk(int max_order_quantity, double max_order_notional)
    : m_max_order_quantity(max_order_quantity),
      m_max_order_notional(max_order_notional) {}

RiskDecision PreTradeRisk::check(const OrderIntent& intent) const {
    // 校验顺序固定，不可调换：先校验正负再计算金额，规避负负相乘干扰判断

    // 1. 价格非法拦截（NaN/Inf/非有限值 或 price <= 0）
    if (!std::isfinite(intent.price) || intent.price <= 0) {
        return RiskDecision::InvalidPrice;
    }

    // 2. 数量非正数拦截
    if (intent.quantity <= 0) {
        return RiskDecision::NonPositiveQuantity;
    }

    // 3. 数量超出单笔上限（上限为包含等于的 inclusive limit，等于上限允许通过）
    if (intent.quantity > m_max_order_quantity) {
        return RiskDecision::QuantityLimitExceeded;
    }

    // 4. 名义金额 = 价格 × 数量，超出上限拦截
    double notional = intent.price * static_cast<double>(intent.quantity);
    if (notional > m_max_order_notional) {
        return RiskDecision::NotionalLimitExceeded;
    }

    return RiskDecision::Accepted;  // 全部校验通过
}
