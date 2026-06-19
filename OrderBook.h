#pragma once
#include <functional>  //std::greater<T>
#include <list>
#include <map>
#include <unordered_map>
#include <vector>

#include "Order.h"

class OrderBook {
public:
    void add_order(const Order& order);  // 添加订单函数（无论卖单还是买单）
    void print_book() const;             // 打印订单薄
    std::vector<Fill>
    match(const Order& order);  // 撮合函数（买盘吃卖盘、卖盘吃买盘）
    bool cancel(uint64_t id);   // 取消、撤回订单

private:
    template <typename BookMap>  // 模板函数（封装原买方-卖方while循环）
    void match_against(BookMap& book_side, const Order& taker, int& remaining,
                       std::vector<Fill>& fills);
    std::map<double, std::list<Order>,
             std::greater<double>>
        m_bids;  // 买盘，价高优先(降序) <price, order_list>
    std::map<double, std::list<Order>>
        m_asks;  // 卖盘，价低优先（升序）<price, order_list>
    std::unordered_map<uint64_t,
                       std::list<Order>::iterator>
        m_order_index;  // 所有订单索引副本 <id, order_list_iterator>
};