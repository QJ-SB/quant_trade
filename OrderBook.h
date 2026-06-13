#pragma once
#include <functional>  //std::greater<T>
#include <list>
#include <map>

#include "Order.h"

class OrderBook {
public:
    void add_order(const Order& order);  //添加订单函数（无论卖单还是买单）
    void print_book() const;             //打印订单薄

private:
    std::map<double, std::list<Order>, std::greater<double>>
        m_bids;                                 // 买盘，价高优先(降序)
    std::map<double, std::list<Order>> m_asks;  // 卖盘，价低优先（升序）
};