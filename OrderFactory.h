// 订单工厂：将策略层的订单意图转为带唯一ID的正式订单，对外提供ID自增管理
#pragma once

#include <cstdint>  // uint64_t

#include "Order.h"
#include "OrderIntent.h"

class OrderFactory {
public:
    explicit OrderFactory(uint64_t first_id);

    Order create_order(const OrderIntent& intent);  // 意图转换为订单

private:
    uint64_t m_next_id;
};
