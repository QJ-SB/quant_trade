#include <iostream>
#include <string>
#include <vector>

#include "Order.h"
#include "Tick.h"


//辅助函数,强枚举类型 → 字符串:
std::string direction_to_string(Direction d) {
    if (d == Direction::Buy)
        return "Buy";
    return "Sell";
}

int main() {

    //构造Tick类实例
    Tick tick1(100.5, 50);
    Tick tick2(100.6, 51);
    Tick tick3(100.7, 52);

    //装进vector
    std::vector<Tick> ticks;
    ticks.push_back(tick1);
    ticks.push_back(tick2);
    ticks.push_back(tick3);

    //循环打印Ticks
    for (const Tick& tick : ticks) {
        std::cout << "价格：" << tick.get_price() << "  ";
        std::cout << "成交量：" << tick.get_volume() << "  ";
        std::cout << "成交额：" << tick.get_turnover() << std::endl;
    }

    //构造Order类实例
    Order order1(99.1, 10, Direction::Buy);
    Order order2(99.2, 20, Direction::Buy);
    Order order3(100.7, 30, Direction::Sell);

    //装进vector
    std::vector<Order> orders;
    orders.push_back(order1);
    orders.push_back(order2);
    orders.push_back(order3);

    //循环打印Orders
    for (const Order& order : orders) {
        std::cout << "委托价格：" << order.get_price() << "  ";
        std::cout << "委托数量：" << order.get_quantity() << "  ";
        std::cout << "交易方向：" << direction_to_string(order.get_direction())
                  << std::endl;
    }


    return 0;
}