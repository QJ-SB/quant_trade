// 策略层输出：订单意图（方向、价格、数量），由上层引擎负责实例化为真实Order
#pragma once

#include "Order.h"

struct OrderIntent {
    OrderDirection direction;
    double price;
    int quantity;
};
