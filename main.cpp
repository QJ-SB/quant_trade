#include <iostream>
#include <vector>

#include "Order.h"
#include "OrderManager.h"
#include "Tick.h"


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
    Order order1(128459023748920347, Direction::Buy, 99.1, 10);
    Order order2(128459023748920348, Direction::Buy, 99.2, 20);
    Order order3(128459023748920349, Direction::Sell, 100.7, 30);

    //装进vector
    std::vector<Order> orders;
    orders.push_back(order1);
    orders.push_back(order2);
    orders.push_back(order3);

    //循环打印Orders
    for (const Order& order : orders) {
        std::cout << "订单编号：" << order.get_id() << "  ";
        std::cout << "订单状态：" << order_status_to_string(order.get_status())
                  << "  ";
        std::cout << "交易方向：" << direction_to_string(order.get_direction())
                  << "  ";
        std::cout << "委托价格：" << order.get_price() << "  ";
        std::cout << "委托数量：" << order.get_quantity() << std::endl;
    }

    //验证OrderManager
    std::cout << "=======验证OrderManager添加功能=======" << std::endl;
    OrderManager order_manager;
    order_manager.add_order(order1);
    order_manager.add_order(order2);
    order_manager.add_order(order3);
    std::cout << "容器内订单数量：" << order_manager.get_order_size()
              << std::endl;
    std::cout << "遍历打印容器内订单：" << std::endl;
    order_manager.print_order();

    //验证OrderManager查询功能
    std::cout << "=======验证OrderManager查询功能=======" << std::endl;
    auto ret1 = order_manager.get_order(order1.get_id());
    if (ret1 != std::nullopt) {
        std::cout << "订单编号：" << ret1->get_id() << "  ";
        std::cout << "订单状态：" << order_status_to_string(ret1->get_status())
                  << "  ";
        std::cout << "交易方向：" << direction_to_string(ret1->get_direction())
                  << "  ";
        std::cout << "委托价格：" << ret1->get_price() << "  ";
        std::cout << "委托数量：" << ret1->get_quantity() << std::endl;
    } else {
        std::cout << "查无此单" << std::endl;
    }
    auto ret2 = order_manager.get_order(123);
    if (ret2 != std::nullopt) {
        std::cout << "订单编号：" << ret2->get_id() << "  ";
        std::cout << "订单状态：" << order_status_to_string(ret2->get_status())
                  << "  ";
        std::cout << "交易方向：" << direction_to_string(ret2->get_direction())
                  << "  ";
        std::cout << "委托价格：" << ret2->get_price() << "  ";
        std::cout << "委托数量：" << ret2->get_quantity() << std::endl;
    } else {
        std::cout << "查无此单" << std::endl;
    }

    return 0;
}