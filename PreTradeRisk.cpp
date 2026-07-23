#include "PreTradeRisk.h"

#include <cmath>  // std::isfinite

PreTradeRisk::PreTradeRisk(int max_order_quantity, double max_order_notional)
    : m_max_order_quantity(max_order_quantity),
      m_max_order_notional(max_order_notional) {}

RiskDecision PreTradeRisk::check(const OrderIntent& intent) const {
    // 校验顺序固定不可调换：必须先在正负校验之后才计算金额，
    // 否则负价格 × 负数量的"假正值"会绕过非法参数检查

    // isfinite 同时覆盖 NaN、±Inf，与 price <= 0 一起构成完整的价格合法性检查
    if (!std::isfinite(intent.price) || intent.price <= 0) {
        return RiskDecision::InvalidPrice;
    }

    if (intent.quantity <= 0) {
        return RiskDecision::NonPositiveQuantity;
    }

    // 上限为 inclusive limit，等于上限允许通过
    if (intent.quantity > m_max_order_quantity) {
        return RiskDecision::QuantityLimitExceeded;
    }

    // static_cast 明确将 int 提升为 double，避免隐式算术转换的歧义
    double notional = intent.price * static_cast<double>(intent.quantity);
    if (notional > m_max_order_notional) {
        return RiskDecision::NotionalLimitExceeded;
    }

    return RiskDecision::Accepted;
}
