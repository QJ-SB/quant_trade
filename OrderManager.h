#pragma once
#include <cstdint>
#include <optional>
#include <unordered_map>

#include "Order.h"

class OrderManager {
public:
    /// 向管理容器插入订单，若 id 重复则插入失败
    /// @return false 表示 id 已存在，订单未插入
    bool add_order(const Order& ord);

    uint64_t get_order_size() const;

    void print_order() const;

    /// 按 id 查询订单
    /// @return 若 id 不存在则返回 std::nullopt
    std::optional<Order> get_order_content(uint64_t id) const;

    /// 按 id 更新订单状态
    /// @return false 表示 id 不存在
    bool update_order_status(uint64_t id, OrderStatus status);

    /// 按 id 扣减订单数量并自动判定 FILLED / PARTIALLY_FILLED 状态
    /// @return false 表示 id 不存在
    bool apply_fill(uint64_t id, int amount);

private:
    std::unordered_map<uint64_t, Order>
        m_orders;  // 默认构造即空容器，无需显式初始化
};
