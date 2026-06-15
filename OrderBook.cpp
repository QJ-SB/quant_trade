#include "OrderBook.h"

#include <algorithm>  //std::min
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

int OrderBook::match(const Order& order) {
    int remaining = order.get_quantity();  // 局部的买单余量跟踪

    // while循环：
    // 【买单吃卖盘】：买单有余量 && 卖盘非空 && 买单价>=卖盘最低价（成交价）
    while (remaining > 0 && !m_asks.empty() &&
           order.get_price() >= m_asks.begin()->first) {
        auto asks_it = m_asks.begin();  // 拿到卖盘起始迭代器
        auto& maker =
            asks_it->second.front();  // 拿到卖盘起始单的“队头”（挂单者）

        int fill =
            std::min(remaining, maker.get_quantity());  // 装填“成交量缓冲区”
        double trade_price = asks_it->first;  // 成交价 = 卖盘“best ask”
        std::cout << "成交  价：" << trade_price << " 量：" << fill
                  << " (taker:" << order.get_id() << " maker:" << maker.get_id()
                  << ")"
                  << std::endl;  // 打印 ———— maker：挂单者，taker：吃单者

        maker.reduce_quantity(fill);  // 卖盘队头-扣减成交量
        remaining -= fill;            // 新单余量-跟踪更新

        if (0 == maker.get_quantity()) {    // 如果队头被吃完
            asks_it->second.pop_front();    // 队头出队，后续补队头
            if (asks_it->second.empty()) {  // 没有队员了
                m_asks.erase(asks_it);      // 卖盘移除节点
            }
        }
    }

    // 循环吃单结束后：
    if (remaining > 0) {  // 如果买单没吃完卖盘
        Order leftover = order;  // 拷贝构造“新单拷贝”（因为新单是const）
        leftover.set_quantity(remaining);  // 撮合余量-回挂-新单拷贝
        add_order(leftover);  // 调用orderbook内部添加新单职能
    }

    return remaining;  // 最后返回撮合后新单余量
}
