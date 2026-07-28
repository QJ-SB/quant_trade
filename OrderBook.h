#pragma once
#include <functional>  // std::greater<T>
#include <list>
#include <map>
#include <unordered_map>
#include <vector>

#include "Fill.h"
#include "Order.h"

class OrderBook {
public:
    /// 将订单按方向挂入 bids 或 asks，同价位订单按进入顺序排列。
    ///
    /// 调用方必须保证订单 id 尚未存在；重复 id 会触发内部不变量断言。
    void add_order(const Order& order);

    void print_book() const;

    /// 以传入订单作为 taker 进行撮合：Buy 吃 asks，Sell 吃 bids。
    ///
    /// 成交价使用 maker 侧价格；未成交余量会重新挂入订单簿。
    /// @return 本次撮合产生的所有 fills。
    std::vector<Fill> match(const Order& order);

    /// 按 id 撤销订单。
    /// @return true 表示撤单成功；false 表示 id 不存在，订单簿保持不变。
    bool cancel(uint64_t id);

private:
    template <typename BookMap>
    void match_against(BookMap& book_side, const Order& taker, int& remaining,
                       std::vector<Fill>& fills);

    std::map<double, std::list<Order>,
             std::greater<double>>
        m_bids;  // price 降序，同价位按 time priority 排列
    std::map<double, std::list<Order>>
        m_asks;  // price 升序，同价位按 time priority 排列
    std::unordered_map<uint64_t,
                       std::list<Order>::iterator>
        m_order_index;  // id → list iterator，用于按 id 定位订单
};
