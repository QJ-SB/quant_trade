#pragma once
#include <vector>

#include "Order.h"  //Fill结构体
#include "OrderBook.h"
#include "OrderManager.h"

class Exchange {
public:
    std::vector<Fill> submit_order(const Order& order);  //新订单提交入口
    const OrderManager& get_order_manager() const;  //对外返回const OMS引用
    const OrderBook& get_order_book() const;  //对外返回const OrderBook引用

private:
    OrderManager m_order_manager;  // OMS系统
    OrderBook m_order_book;        // 订单簿系统
};
