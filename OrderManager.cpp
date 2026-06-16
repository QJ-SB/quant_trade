#include "OrderManager.h"

#include <cassert>  //assert()
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
    if (it != m_orders.end()) {  //如果订单存在（继续）
        assert(amount <= it->second.get_quantity());  //断言检查（兜底）
        it->second.reduce_quantity(amount);  //原始订单扣减撮合量
        if (0 == it->second.get_quantity()) {  //如果原始订单全部扣完————FILLED
            it->second.set_status(OrderStatus::FILLED);
        } else {  //如果还有剩余————PARTIALLY_FILLED
            it->second.set_status(OrderStatus::PARTIALLY_FILLED);
        }
        return true;
    } else {  //如果订单不存在（返回错误）
        return false;
    }
}
