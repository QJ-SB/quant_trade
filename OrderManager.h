#pragma once

#include <cstdint>        //订单编号：uint64_t
#include <unordered_map>  //哈希表容器

#include "Order.h"

class OrderManager {
public:
    OrderManager() = default;  //默认无参构造

    bool add_order(const Order& ord);  //添加订单函数（const引用避免修改和构造）
    uint64_t get_order_size() const;  //查询订单数函数
    void print_order() const;         //变量打印订单函数

private:
    std::unordered_map<uint64_t, Order>
        m_orders;  //订单管理容器（创建时自动置空）
};