#include "OrderBook.h"

#include <iostream>


void OrderBook::add_order(const Order& order) {
    if (order.get_direction() == OrderDirection::Buy) {
        m_bids[order.get_price()].push_back(order);
    } else {
        m_asks[order.get_price()].push_back(order);
    }
}

void OrderBook::print_book() const {
    std::cout << "=============【订单簿】==============" << std::endl;
    std::cout << "=====================================" << std::endl;
    std::cout << "【卖盘】：" << std::endl;
    for (auto it = m_asks.rbegin(); it != m_asks.rend(); ++it) {
        for (const auto& item : it->second) {
            std::cout << item.get_price() << " x " << item.get_quantity()
                      << "  订单id:" << item.get_id() << std::endl;
        }
    }
    std::cout << "—————————————————————————————————————" << std::endl;
    std::cout << "【买盘】：" << std::endl;
    for (const auto& pair : m_bids) {
        for (const auto& item : pair.second) {
            std::cout << item.get_price() << " x " << item.get_quantity()
                      << "  订单id:" << item.get_id() << std::endl;
        }
    }
}
