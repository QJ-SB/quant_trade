#pragma once

#include "Order.h"

/// 策略层输出的订单意图，只描述方向、价格与数量；
/// order id 与初始状态由 OrderFactory 创建真实 Order 时补充。
struct OrderIntent {
    OrderDirection direction;
    double price;
    int quantity;
};
