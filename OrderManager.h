#pragma once
#include <cstdint>
#include <optional>
#include <unordered_map>

#include "Order.h"

/// 维护订单的权威状态，并提供登记、查询、状态更新与成交回写。
class OrderManager {
public:
    /// 登记订单。
    /// @return false 表示 id 已存在，原订单保持不变。
    bool add_order(const Order& ord);

    uint64_t get_order_size() const;

    void print_order() const;

    /// 按 id 查询订单，返回当前状态的副本。
    /// @return 若 id 不存在则返回 std::nullopt。
    std::optional<Order> get_order_content(uint64_t id) const;

    /// 按 id 更新订单状态。
    /// @return false 表示 id 不存在。
    bool update_order_status(uint64_t id, OrderStatus status);

    /// 扣减订单的剩余数量，并更新为 FILLED 或 PARTIALLY_FILLED。
    /// @return false 表示 id 不存在。
    bool apply_fill(uint64_t id, int amount);

private:
    std::unordered_map<uint64_t, Order> m_orders;
};
