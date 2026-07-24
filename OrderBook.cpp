#include "OrderBook.h"

#include <algorithm>  //std::min
#include <cassert>    //assert()
#include <iostream>
#include <vector>

#include "Order.h"

/// `BookMap` 为 bids 或 asks 的 map 类型，`key_comp()` 利用两侧相反的
/// 排序比较器复用同一撮合循环。
/// @param book_side 当前被 taker 撮合的一侧订单簿
/// @param taker 本次主动进入订单簿的订单
/// @param remaining taker 当前尚未成交的数量，循环体内持续扣减
/// @param fills 收集本次撮合产生的成交记录
template <typename BookMap>
void OrderBook::match_against(BookMap& book_side, const Order& taker,
                              int& remaining, std::vector<Fill>& fills) {
    // remaining > 0：taker 仍有余量可撮合
    // !book_side.empty() 位于访问 begin() 之前，依赖 && 短路防止空容器解引用
    // key_comp() 条件同时适配 Buy 吃 asks（less）与 Sell 吃 bids（greater）
    while (remaining > 0 && !book_side.empty() &&
           !book_side.key_comp()(taker.get_price(), book_side.begin()->first)) {
        auto it = book_side.begin();  // 当前最优价格档（price-time priority）
        auto& maker = it->second.front();  // 该价位最早进入的 maker

        int fill_amount = std::min(remaining, maker.get_quantity());
        double trade_price = it->first;
        // 成交价使用 maker 挂单价
        std::cout << "【成交】" << std::endl;
        std::cout << "价：" << trade_price << " 量：" << fill_amount
                  << " (taker:" << taker.get_id() << " maker:" << maker.get_id()
                  << ")" << std::endl;

        maker.reduce_quantity(fill_amount);
        remaining -= fill_amount;

        Fill fill_info;
        fill_info.taker_id = taker.get_id();
        fill_info.maker_id =
            maker.get_id();  // 必须在 pop_front() 前读取：pop_front() 后 maker
                             // 引用和对应 iterator 失效
        fill_info.quantity = fill_amount;
        fill_info.price = trade_price;
        fills.push_back(fill_info);

        // maker 完全成交后执行清理：先删索引，再移出价位队列，队列为空时删 map
        // 节点
        if (0 == maker.get_quantity()) {
            m_order_index.erase(fill_info.maker_id);  // 1. 从索引中移除
            it->second.pop_front();  // 2. 从价位队列移除
            if (it->second.empty())  // 3. 价位队列已空
                book_side.erase(it);  //    删 map 节点，维持索引与订单簿一致性
        }
    }
}

void OrderBook::add_order(const Order& order) {
    double key = order.get_price();

    // Buy 与 Sell 分支对称，以下以 Buy 说明共同机制
    if (order.get_direction() == OrderDirection::Buy) {
        auto& lst = m_bids[key];  // operator[] 在价格档不存在时创建该档位
        // 尾插维持同价位 time priority；返回的 iterator 存入索引供撤单定位
        auto ret = lst.insert(lst.end(), order);
        [[maybe_unused]] auto result = m_order_index.insert(
            {order.get_id(), ret});  // id → list iterator，供 cancel 按 id 查找
        // 内部不变量守卫：调用方必须在进入 OrderBook 前拒绝重复 id
        assert(result.second);

    } else {
        auto& lst = m_asks[key];
        auto ret = lst.insert(lst.end(), order);
        [[maybe_unused]] auto result =
            m_order_index.insert({order.get_id(), ret});
        assert(result.second);
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
    // order 作为 taker 进入撮合：Buy 吃 asks，Sell 吃 bids
    // remaining 独立跟踪 taker 未成交量，因为 order 是 const
    int remaining = order.get_quantity();
    std::vector<Fill> fills;

    if (order.get_direction() == OrderDirection::Buy) {
        match_against(m_asks, order, remaining, fills);
    } else {
        match_against(m_bids, order, remaining, fills);
    }

    // 撮合后仍有剩余：拷贝 order 并挂回订单簿
    // 挂回后该订单从 taker 转为后续撮合中的 maker
    if (remaining > 0) {
        Order leftover = order;
        leftover.set_quantity(remaining);
        add_order(leftover);
    }

    return fills;
}

bool OrderBook::cancel(uint64_t id) {
    // m_order_index 是撤单入口的 membership 真相源
    // 未命中返回 false 是正常的运行时结果，不是内部错误
    auto idx_map_it = m_order_index.find(id);
    if (idx_map_it == m_order_index.end())
        return false;

    // 必须通过方向和价格反查 bids/asks 价位：list::iterator
    // 本身不携带所属容器信息，无法脱离其所在 list 执行 erase
    auto order_list_it = idx_map_it->second;

    // Buy 与 Sell 分支对称，以下以 Buy 说明共同机制
    if (order_list_it->get_direction() == OrderDirection::Buy) {
        double key = order_list_it->get_price();

        // 使用 find() 而非 operator[]：撤单路径不应在 key 不存在时创建空价位
        auto map_it = m_bids.find(key);
        assert(map_it != m_bids.end());  // 索引与订单簿一致性守卫
        map_it->second.erase(order_list_it);

        // 删除订单后清理空价位，避免订单簿保留无订单的 price level
        if (map_it->second.empty())  // 价位已空
            m_bids.erase(map_it);    // 删 map 节点
    } else {
        double key = order_list_it->get_price();
        auto map_it = m_asks.find(key);

        // 内部不变量守卫：索引中的订单必须仍存在于对应价格档
        assert(map_it != m_asks.end());
        map_it->second.erase(order_list_it);
        if (map_it->second.empty())
            m_asks.erase(map_it);
    }

    // 簿内节点清理完成后，同步移除 membership 索引
    m_order_index.erase(id);
    return true;
}
