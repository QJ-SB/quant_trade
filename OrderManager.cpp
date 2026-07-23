#include "OrderManager.h"

#include <cassert>
#include <iostream>

#include "Order.h"


bool OrderManager::add_order(const Order& ord) {
    auto ret = m_orders.insert({ord.get_id(), ord});
    return ret.second;
}

uint64_t OrderManager::get_order_size() const {
    return static_cast<uint64_t>(m_orders.size());
}

void OrderManager::print_order() const {
    for (const auto& [id, ord] : m_orders) {
        std::cout << "订单编号：" << id << "  ";
        std::cout << "订单状态：" << order_status_to_string(ord.get_status())
                  << "  ";
        std::cout << "订单方向："
                  << order_direction_to_string(ord.get_direction()) << "  ";
        std::cout << "委托价格：" << ord.get_price() << "  ";
        std::cout << "委托数量：" << ord.get_quantity() << std::endl;
    }
}

std::optional<Order> OrderManager::get_order_content(uint64_t id) const {
    auto it = m_orders.find(id);
    if (it != m_orders.end()) {
        return it->second;
    } else {
        return std::nullopt;
    }
}

bool OrderManager::update_order_status(uint64_t id, OrderStatus status) {
    auto it = m_orders.find(id);
    if (it != m_orders.end()) {
        it->second.set_status(status);
        return true;
    } else {
        return false;
    }
}

bool OrderManager::apply_fill(uint64_t id, int amount) {
    auto it = m_orders.find(id);
    if (it != m_orders.end()) {
        // assert 内部不变量守卫：成交量不得超过 OMS 中的剩余量
        assert(amount <= it->second.get_quantity());
        it->second.reduce_quantity(amount);
        // 常量放左侧避免误写成 if (quantity = 0) 的赋值陷阱
        if (0 == it->second.get_quantity()) {
            it->second.set_status(OrderStatus::FILLED);
        } else {
            it->second.set_status(OrderStatus::PARTIALLY_FILLED);
        }
        return true;
    } else {
        return false;
    }
}
