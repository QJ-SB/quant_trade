// Order.h
#pragma once

enum class Direction { Buy, Sell };  // 交易方向，用强枚举类型

class Order {
public:
    Order(double price, int quantity, Direction direction);

    double get_price() const;
    int get_quantity() const;
    Direction get_direction() const;

private:
    double m_price;         //委托价格
    int m_quantity;         //委托数量
    Direction m_direction;  //交易方向
};