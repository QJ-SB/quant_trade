// 盘前风控：对策略层输出的订单意图进行可配置上限校验，校验通过方可流入订单工厂
#pragma once

#include "OrderIntent.h"

// 风控决策枚举：逐项区分全部拒绝原因，不使用 bool 或 optional 承载
enum class RiskDecision {
    Accepted,  // 通过

    InvalidPrice,         // 价格非法（price <= 0 或 price 非有限值）
    NonPositiveQuantity,  // 数量非正数（quantity <= 0）

    QuantityLimitExceeded,  // 数量超出单笔上限（quantity > max_order_quantity）

    NotionalLimitExceeded  // 名义金额超出单笔上限
                           //（notional > max_order_notional）
};

class PreTradeRisk {
public:
    // 功能描述：构造盘前风控实例，配置单笔数量上限与名义金额上限
    // 参数说明：max_order_quantity — 单笔最大数量（含等于），
    //          max_order_notional — 单笔最大名义金额（含等于）
    // 返回值说明：(无)
    // 异常说明：(无)
    PreTradeRisk(int max_order_quantity, double max_order_notional);

    // 功能描述：对订单意图执行固定顺序的盘前风控校验
    // 参数说明：intent — 策略层输出的订单意图（方向、价格、数量）
    // 返回值说明：RiskDecision 枚举，Accepted 表示通过，其他值区分具体拒绝原因
    // 异常说明：(无)
    RiskDecision check(const OrderIntent& intent) const;

private:
    int m_max_order_quantity;     // 单笔最大数量上限
    double m_max_order_notional;  // 单笔最大名义金额上限
};
