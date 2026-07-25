#pragma once

#include <cstdint>

#include "Order.h"
#include "OrderIntent.h"

/// 将 OrderIntent 转换为 Order，并在当前实例内分配从 first_id
/// 开始连续递增的 order id。
class OrderFactory {
public:
    explicit OrderFactory(uint64_t first_id);

    Order create_order(const OrderIntent& intent);

private:
    uint64_t m_next_id;
};
