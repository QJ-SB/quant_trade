// 盘前风控：对策略层输出的订单意图进行可配置上限校验，校验通过方可流入订单工厂
#pragma once

#include "OrderIntent.h"

// 风控决策枚举：逐项区分全部拒绝原因，不使用 bool 或 optional 承载
enum class RiskDecision {
    Accepted,

    InvalidPrice,         // price <= 0 或非有限值
    NonPositiveQuantity,  // quantity <= 0

    QuantityLimitExceeded,  // quantity > 单笔上限

    NotionalLimitExceeded  // notional > 单笔上限
};

class PreTradeRisk {
public:
    /// 构造风控实例并配置两项上限（均为 inclusive limit，等于上限允许通过）
    /// @param max_order_quantity 单笔最大数量
    /// @param max_order_notional 单笔最大名义金额
    PreTradeRisk(int max_order_quantity, double max_order_notional);

    /// 按固定顺序对订单意图执行风控校验
    /// @return Accepted 表示放行，其他值标识具体拒绝原因
    RiskDecision check(const OrderIntent& intent) const;

private:
    int m_max_order_quantity;
    double m_max_order_notional;
};
