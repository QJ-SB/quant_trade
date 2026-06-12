// Order.h
#pragma once

#include <cstdint>  //订单编号：uint64_t

enum class Direction { Buy, Sell };  //交易方向，用强枚举类型【类外定义】

enum class OrderStatus {  //订单状态，用强枚举类型【类外定义】
    PENDING,              //挂单中、待成交
    FILLED,               //已成交
    CANCELLED,            //已撤
    REJECTED              //被拒
};

class Order {
public:
    //构造函数
    Order(uint64_t id, Direction direction, double price, int quantity);

    uint64_t get_id() const;          //订单id-getter
    OrderStatus get_status() const;   //订单状态-getter
    Direction get_direction() const;  //交易方向-getter
    double get_price() const;         //委托价格-getter
    int get_quantity() const;         //委托数量-getter

private:
    uint64_t m_order_id;                                //订单编号
    OrderStatus m_order_status = OrderStatus::PENDING;  //订单状态(默认挂单)
    Direction m_direction;                              //交易方向
    double m_price;                                     //委托价格
    int m_quantity;                                     //委托数量
};