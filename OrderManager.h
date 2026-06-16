#pragma once
#include <cstdint>        //订单编号：uint64_t
#include <optional>       //封装订单查询返回值
#include <unordered_map>  //哈希表容器

#include "Order.h"

class OrderManager {
public:
    bool add_order(const Order& ord);  //添加订单函数（const引用避免修改和构造）
    uint64_t get_order_size() const;  //查询订单数函数
    void print_order() const;         //变量打印订单函数
    std::optional<Order> get_order_content(uint64_t id) const;  //查询订单内容
    bool update_order_status(uint64_t id, OrderStatus status);  //更新订单状态
    bool apply_fill(uint64_t id, int amount);  //根据撮合结果“扣量 + 判状态”

private:
    std::unordered_map<uint64_t, Order>
        m_orders;  //订单管理容器（创建时自动置空）
};