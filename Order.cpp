#include "Order.h"

Order::Order(uint64_t id, Direction direction, double price, int quantity)
    : m_order_id(id), m_direction(direction), m_price(price),
      m_quantity(quantity) {}

uint64_t Order::get_id() const {
    return m_order_id;
}

OrderStatus Order::get_status() const {
    return m_order_status;
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