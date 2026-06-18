#include "OrderBook.h"

#include <algorithm>  //std::min
#include <cassert>    //assert()
#include <iostream>
#include <vector>

#include "Order.h"


void OrderBook::add_order(const Order& order) {
    double key = order.get_price();  //统一拿到无论是bids还是asks的[key]

    if (order.get_direction() == OrderDirection::Buy) {  //买方
        auto& lst = m_bids[key];                         //查一次，用两次
        auto ret =
            lst.insert(lst.end(), order);  //找到list，然后尾插,并返回迭代器
        m_order_index[order.get_id()] = ret;  // list<Order>迭代器存入 O(1)索引

    } else {  //卖方
        auto& lst = m_asks[key];
        auto ret = lst.insert(lst.end(), order);
        m_order_index[order.get_id()] = ret;
    }
}

void OrderBook::print_book() const {
    std::cout << "=============【订单簿】==============" << std::endl;
    std::cout << "=====================================" << std::endl;
    std::cout << "【卖盘】" << std::endl;
    for (auto it = m_asks.rbegin(); it != m_asks.rend(); ++it) {
        for (const auto& item : it->second) {
            std::cout << item.get_price() << " x " << item.get_quantity()
                      << "  订单id:" << item.get_id() << std::endl;
        }
    }
    std::cout << "—————————————————————————————————————" << std::endl;
    std::cout << "【买盘】" << std::endl;
    for (const auto& pair : m_bids) {
        for (const auto& item : pair.second) {
            std::cout << item.get_price() << " x " << item.get_quantity()
                      << "  订单id:" << item.get_id() << std::endl;
        }
    }
    std::cout << "=====================================" << std::endl;
    std::cout << "=====================================" << std::endl;
}

std::vector<Fill> OrderBook::match(const Order& order) {
    //【共用部分】
    int remaining = order.get_quantity();  // 局部的买、卖单余量跟踪
    std::vector<Fill> fills;               //记录返回的成交信息

    //【买吃卖】如果是新单是买单
    if (order.get_direction() == OrderDirection::Buy) {
        // while循环1：
        // 【买单吃卖盘】：买单有余量 && 卖盘非空 &&
        // 买单价>=卖盘最低价（成交价）
        while (remaining > 0 && !m_asks.empty() &&
               order.get_price() >= m_asks.begin()->first) {
            auto asks_it = m_asks.begin();  // 拿到卖盘起始迭代器
            auto& maker =
                asks_it->second.front();  // 拿到卖盘起始单的“队头”（挂单者）

            int fill_amount =
                std::min(remaining,
                         maker.get_quantity());  // 装填“成交量缓冲区”
            double trade_price = asks_it->first;  // 成交价 = 卖盘“best ask”
            std::cout << "成交  价：" << trade_price << " 量：" << fill_amount
                      << " (taker:" << order.get_id()
                      << " maker:" << maker.get_id() << ")"
                      << std::endl;  // 打印 ———— maker：挂单者，taker：吃单者

            maker.reduce_quantity(fill_amount);  // 卖盘队头-扣减成交量
            remaining -= fill_amount;            // 新单余量-跟踪更新

            Fill fill_info;  //填充此级撮合的成交信息（特意在pop_front前填充）
            fill_info.taker_id = order.get_id();
            fill_info.maker_id = maker.get_id();
            fill_info.quantity = fill_amount;
            fill_info.price = trade_price;
            fills.push_back(fill_info);  // push到返回成交信息的容器里

            if (0 == maker.get_quantity()) {  // 如果队头被吃完
                // 删除list队头的本地索引（注意别用maker，它要出队）
                m_order_index.erase(fill_info.maker_id);
                asks_it->second.pop_front();  // 队头出队，后续补队头
                if (asks_it->second.empty())  // 没有队员了
                    m_asks.erase(asks_it);    // 卖盘移除节点
            }
        }
    } else {  // 【卖吃买】如果是新单是卖单
        // while循环2：
        // 【卖单吃买盘】：卖单有余量 && 买盘非空 &&
        // 卖单价<=买盘最高价（成交价）
        while (remaining > 0 && !m_bids.empty() &&
               order.get_price() <= m_bids.begin()->first) {
            auto bids_it = m_bids.begin();  // 拿到买盘起始迭代器
            auto& maker =
                bids_it->second.front();  // 拿到买盘起始单的“队头”（挂单者）

            int fill_amount =
                std::min(remaining,
                         maker.get_quantity());  // 装填“成交量缓冲区”
            double trade_price = bids_it->first;  // 成交价 = 买盘“best bid”
            std::cout << "成交  价：" << trade_price << " 量：" << fill_amount
                      << " (taker:" << order.get_id()
                      << " maker:" << maker.get_id() << ")"
                      << std::endl;  // 打印 ———— maker：挂单者，taker：吃单者

            maker.reduce_quantity(fill_amount);  // 买盘队头-扣减成交量
            remaining -= fill_amount;            // 新单余量-跟踪更新

            Fill fill_info;  //填充此级撮合的成交信息（特意在pop_front前填充）
            fill_info.taker_id = order.get_id();
            fill_info.maker_id = maker.get_id();
            fill_info.quantity = fill_amount;
            fill_info.price = trade_price;
            fills.push_back(fill_info);  // push到返回成交信息的容器里

            if (0 == maker.get_quantity()) {  // 如果队头被吃完
                // 删除list队头的本地索引（注意别用maker，它要出队）
                m_order_index.erase(fill_info.maker_id);
                bids_it->second.pop_front();  // 队头出队，后续补队头
                if (bids_it->second.empty()) {  // 没有队员了
                    m_bids.erase(bids_it);      // 买盘移除节点
                }
            }
        }
    }

    // 【共用部分】循环吃单结束后：
    // 若撮合有剩余：
    if (remaining > 0) {  // 如果新的买、卖单-没吃完-卖、买盘
        Order leftover = order;  // 拷贝构造“新单拷贝”（因为新单是const）
        leftover.set_quantity(remaining);  // 撮合的余量-回挂-新单拷贝
        add_order(leftover);  // 调用orderbook内部添加新单职能
    }

    // 返回Fill-成交信息：
    return fills;  // 最后返回本次撮合产生的成交记录
}

bool OrderBook::cancel(uint64_t id) {
    auto idx_map_it = m_order_index.find(id);  //先查id存不存在
    if (idx_map_it == m_order_index.end())
        return false;  // id不存在直接返回false，存在👇

    auto order_list_it = idx_map_it->second;  //拿到list<Order>的迭代器
    //进一步判断买卖方向，判断要去asks还是bids里erase
    if (order_list_it->get_direction() == OrderDirection::Buy) {  //买盘
        double key = order_list_it->get_price();  //统一获取m_bids的[key]
        // find()比较兜底，[key]不存在会直接插
        auto map_it = m_bids.find(key);  //通过key找到对应list<Order>迭代器
        assert(map_it != m_bids.end());  //断言保证索引里有，book里肯定有
        map_it->second.erase(order_list_it);  // erase掉list<Order>的迭代器
        if (map_it->second.empty())  //如果该list<Order>被erase完了
            m_bids.erase(map_it);    // erase掉m_bids的迭代器
    } else {                         //卖盘
        double key = order_list_it->get_price();  //统一获取m_asks的[key]
        auto map_it = m_asks.find(key);
        assert(map_it != m_asks.end());
        map_it->second.erase(order_list_it);
        if (map_it->second.empty())
            m_asks.erase(map_it);
    }
    //最后统一删除维护的哈希索引副本
    m_order_index.erase(id);
    return true;
}
