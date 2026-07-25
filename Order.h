#pragma once
#include <cstdint>
#include <string>

enum class OrderDirection { Buy, Sell };

enum class OrderStatus {
    PENDING,
    FILLED,
    PARTIALLY_FILLED,
    CANCELLED,
    REJECTED
};

/// 记录一次撮合成交；quantity 同时作用于 taker 与 maker，
/// price 取 maker 的挂单价格。
struct Fill {
    uint64_t taker_id;
    uint64_t maker_id;
    int quantity;
    double price;
};

std::string order_direction_to_string(OrderDirection d);

std::string order_status_to_string(OrderStatus o);

/// 表示一笔订单及其当前状态与剩余数量。
class Order {
public:
    Order(uint64_t id, OrderDirection direction, double price, int quantity);

    uint64_t get_id() const;
    OrderStatus get_status() const;
    OrderDirection get_direction() const;
    double get_price() const;
    int get_quantity() const;

    void set_status(OrderStatus status);

    /// 从当前剩余数量中扣减成交量；调用方必须保证 amount
    /// 合法且不超过当前剩余数量。
    void reduce_quantity(int amount);

    /// 将订单副本的数量设为未成交残量，用于 taker 残量回挂。
    void set_quantity(int quantity);

private:
    uint64_t m_id;
    OrderStatus m_status = OrderStatus::PENDING;
    OrderDirection m_direction;
    double m_price;
    int m_quantity;
};
