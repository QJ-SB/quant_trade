#include "OrderManager.h"

#include <iostream>


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
        std::cout << "委托价格：" << ord.get_price() << "  ";
        std::cout << "委托数量：" << ord.get_quantity() << std::endl;
    }
}

std::optional<Order> OrderManager::get_order(uint64_t id) const {
    auto it = m_orders.find(id);
    if (it != m_orders.end()) {
        return it->second;
    } else {
        return std::nullopt;
    }
}