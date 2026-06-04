#include "Order.h"

Order::Order(double price, int quantity, Direction direction)
    : m_price(price), m_quantity(quantity), m_direction(direction) {}

double Order::get_price() const {
    return m_price;
}

int Order::get_quantity() const {
    return m_quantity;
}

Direction Order::get_direction() const {
    return m_direction;
}
