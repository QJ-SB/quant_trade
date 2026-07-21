#include "OrderFactory.h"

OrderFactory::OrderFactory(uint64_t first_id) : m_next_id(first_id) {}

Order OrderFactory::create_order(const OrderIntent& intent) {
    Order order(m_next_id, intent.direction, intent.price, intent.quantity);
    m_next_id += 1;
    return order;
}
