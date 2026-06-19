#include "OrderBook.h"

#include <algorithm>  //std::min
#include <cassert>    //assert()
#include <iostream>
#include <vector>

#include "Order.h"

// 模板函数（封装原买方-卖方while循环）- 只在OrderBook.cpp中用
// 模板参数BookMap：订单簿红黑树。外界传入引用&，内部直接修改带出。
template <typename BookMap>
void OrderBook::match_against(BookMap& book_side, const Order& taker,
                              int& remaining, std::vector<Fill>& fills) {
    // map.key_comp()取less、greater函子，operator()重载调用
    // 修改逻辑："a <= b → !(a>b)"👇
    // “买吃卖”自动匹配less比较器，“卖吃买”自动匹配greater比较器
    // 【注意此while循环内“条件1”和“条件2”自动保障“条件3”不为空（防触发UB！）】
    while (remaining > 0 && !book_side.empty() &&
           !book_side.key_comp()(taker.get_price(), book_side.begin()->first)) {
        auto it = book_side.begin();       // 拿到订单簿买盘或卖盘的迭代器
        auto& maker = it->second.front();  // 通过迭代器拿到list<Order>里的队头

        // 成交数量 = min(吃盘单，盘余量)
        // 成交价格 = 盘余量队头价格
        int fill_amount = std::min(remaining, maker.get_quantity());
        double trade_price = it->first;
        // 终端打印成交信息
        std::cout << "成交  价：" << trade_price << " 量：" << fill_amount
                  << " (taker:" << taker.get_id() << " maker:" << maker.get_id()
                  << ")" << std::endl;

        maker.reduce_quantity(
            fill_amount);          // 调用盘内订单内部成员函数扣减成交量
        remaining -= fill_amount;  // 吃盘单余量

        Fill fill_info;  // 填充撮合的“成交信息结构体”
        fill_info.taker_id = taker.get_id();
        fill_info.maker_id =
            maker.get_id();  // 【注意在maker.pop_front()之前，谨防UAF！】
        fill_info.quantity = fill_amount;
        fill_info.price = trade_price;
        fills.push_back(fill_info);  // 装入传入的vector-fills

        // 如果订单簿盘内买单、卖单被“吃完”
        if (0 == maker.get_quantity()) {
            m_order_index.erase(fill_info.maker_id);  // 先删本地索引
            it->second.pop_front();   // 吃空的空队头出队，后续自动升为队头
            if (it->second.empty())   // 没有后续，整个list为空
                book_side.erase(it);  // 直接erase该map节点
        }
    }
}

void OrderBook::add_order(const Order& order) {
    double key = order.get_price();  // 统一拿到无论是bids还是asks的[key]

    if (order.get_direction() == OrderDirection::Buy) {  // 买方
        auto& lst = m_bids[key];                         // 查一次，用两次
        auto ret =
            lst.insert(lst.end(), order);     // 找到list，然后尾插,并返回迭代器
        m_order_index[order.get_id()] = ret;  // list<Order>迭代器存入 O(1)索引

    } else {  // 卖方
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
    // 【共用部分】
    int remaining = order.get_quantity();  // 吃盘单余量跟踪
    std::vector<Fill> fills;               // 装载撮合的成交信息

    // 【买吃卖】新单taker是买单
    if (order.get_direction() == OrderDirection::Buy) {
        // while循环1：
        // 【买单吃卖盘】：买单有余量 && 卖盘非空 &&
        // 买单价>=卖盘最低价（成交价）
        match_against(m_asks, order, remaining, fills);  // 调用内部模板函数
    } else {
        // 【卖吃买】新单taker是卖单
        // while循环2：
        // 【卖单吃买盘】：卖单有余量 && 买盘非空 &&
        // 卖单价<=买盘最高价（成交价）
        match_against(m_bids, order, remaining, fills);  // 调用内部模板函数
    }

    // 【共用部分】循环吃单结束后：
    // 若撮合有剩余：
    if (remaining > 0) {         // 新的买、卖单taker，没撮合消耗光
        Order leftover = order;  // 拷贝构造“新单拷贝”（因为新单是const）
        leftover.set_quantity(remaining);  // taker的余量写入拷贝
        add_order(leftover);               // 调用OrderBook内部添加新单职能
    }

    // 返回Fill-成交信息：
    return fills;  // 最后返回本次撮合产生的成交记录
}

bool OrderBook::cancel(uint64_t id) {
    auto idx_map_it = m_order_index.find(id);  // 先查id存不存在
    if (idx_map_it == m_order_index.end())
        return false;  // id不存在直接返回false，存在👇

    auto order_list_it = idx_map_it->second;  // 拿到list<Order>的迭代器
    // 进一步判断买卖方向，判断要去asks还是bids里erase
    if (order_list_it->get_direction() == OrderDirection::Buy) {  // 买盘
        double key = order_list_it->get_price();  // 统一获取m_bids的[key]
        // find()比较兜底，[key]不存在会直接插
        auto map_it = m_bids.find(key);  // 通过key找到对应list<Order>迭代器
        assert(map_it != m_bids.end());  // 断言保证索引里有，book里肯定有
        map_it->second.erase(order_list_it);  // erase掉list<Order>的迭代器
        if (map_it->second.empty())           // 如果该list<Order>被erase完了
            m_bids.erase(map_it);             // erase掉m_bids的迭代器
    } else {                                  // 卖盘
        double key = order_list_it->get_price();  // 统一获取m_asks的[key]
        auto map_it = m_asks.find(key);
        assert(map_it != m_asks.end());
        map_it->second.erase(order_list_it);
        if (map_it->second.empty())
            m_asks.erase(map_it);
    }
    // 最后统一删除维护的哈希索引副本
    m_order_index.erase(id);
    return true;
}
