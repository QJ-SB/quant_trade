#include "Order.h"

#include <string>

Order::Order(uint64_t id, Direction direction, double price, int quantity)
    : m_id(id), m_direction(direction), m_price(price), m_quantity(quantity) {}

uint64_t Order::get_id() const {
    return m_id;
}

OrderStatus Order::get_status() const {
    return m_status;
}

Direction Order::get_direction() const {
    return m_direction;
}

double Order::get_price() const {
    return m_price;
}

int Order::get_quantity() const {
    return m_quantity;
}

void Order::set_status(OrderStatus status) {
    m_status = status;
}

std::string direction_to_string(Direction d) {
    if (d == Direction::Buy)
        return "Buy";
    return "Sell";
}

std::string order_status_to_string(OrderStatus o) {
    switch (o) {
    case OrderStatus::PENDING:
        return "PENDING";  //挂单中、待成交
    case OrderStatus::FILLED:
        return "FILLED";  //已成交
    case OrderStatus::CANCELLED:
        return "CANCELLED";  //已撤
    case OrderStatus::REJECTED:
        return "REJECTED";  //被拒
    }
    return "UNKNOWN";  //未知
}