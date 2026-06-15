#include <iostream>
#include <vector>

#include "Order.h"
#include "OrderBook.h"
#include "OrderManager.h"
#include "Tick.h"


int main() {

    // 构造Tick类实例
    Tick tick1(100.5, 50);
    Tick tick2(100.6, 51);
    Tick tick3(100.7, 52);

    // 装进vector
    std::vector<Tick> ticks;
    ticks.push_back(tick1);
    ticks.push_back(tick2);
    ticks.push_back(tick3);

    // 循环打印Ticks
    std::cout << "=======循环打印Ticks=======" << std::endl;
    for (const Tick& tick : ticks) {
        std::cout << "价格：" << tick.get_price() << "  ";
        std::cout << "成交量：" << tick.get_volume() << "  ";
        std::cout << "成交额：" << tick.get_turnover() << std::endl;
    }

    // 构造Order类实例
    Order order1(128459023748920347, OrderDirection::Buy, 99.1, 10);
    Order order2(128459023748920348, OrderDirection::Buy, 99.2, 20);
    Order order3(128459023748920349, OrderDirection::Buy, 99.3, 30);
    Order order4(128459023748920350, OrderDirection::Sell, 100.7, 30);
    Order order5(128459023748920351, OrderDirection::Sell, 100.6, 20);
    Order order6(128459023748920352, OrderDirection::Sell, 100.5, 10);

    // 装进vector
    std::vector<Order> orders;
    orders.push_back(order1);
    orders.push_back(order2);
    orders.push_back(order3);
    orders.push_back(order4);
    orders.push_back(order5);
    orders.push_back(order6);

    // 循环打印Orders
    std::cout << std::endl;
    std::cout << "=======循环打印Orders=======" << std::endl;
    for (const Order& order : orders) {
        std::cout << "订单编号：" << order.get_id() << "  ";
        std::cout << "订单状态：" << order_status_to_string(order.get_status())
                  << "  ";
        std::cout << "交易方向：" << direction_to_string(order.get_direction())
                  << "  ";
        std::cout << "委托价格：" << order.get_price() << "  ";
        std::cout << "委托数量：" << order.get_quantity() << std::endl;
    }

    // 验证OrderManager
    std::cout << std::endl;
    std::cout << "=======验证OrderManager添加功能=======" << std::endl;
    OrderManager order_manager;
    order_manager.add_order(order1);
    order_manager.add_order(order2);
    order_manager.add_order(order3);
    std::cout << "容器内订单数量：" << order_manager.get_order_size()
              << std::endl;
    std::cout << "遍历打印容器内订单：" << std::endl;
    order_manager.print_order();

    // 验证OrderManager查询功能
    std::cout << std::endl;
    std::cout << "=======验证OrderManager查询功能=======" << std::endl;
    auto ret1 = order_manager.get_order_content(order1.get_id());
    if (ret1 != std::nullopt) {
        std::cout << "订单编号：" << ret1->get_id() << "  ";
        std::cout << "订单状态：" << order_status_to_string(ret1->get_status())
                  << "  ";
        std::cout << "交易方向：" << direction_to_string(ret1->get_direction())
                  << "  ";
        std::cout << "委托价格：" << ret1->get_price() << "  ";
        std::cout << "委托数量：" << ret1->get_quantity() << std::endl;
    } else {
        std::cout << "查无此单！" << std::endl;
    }
    auto ret2 = order_manager.get_order_content(123);
    if (ret2 != std::nullopt) {
        std::cout << "订单编号：" << ret2->get_id() << "  ";
        std::cout << "订单状态：" << order_status_to_string(ret2->get_status())
                  << "  ";
        std::cout << "交易方向：" << direction_to_string(ret2->get_direction())
                  << "  ";
        std::cout << "委托价格：" << ret2->get_price() << "  ";
        std::cout << "委托数量：" << ret2->get_quantity() << std::endl;
    } else {
        std::cout << "查无此单！" << std::endl;
    }

    // 验证OrderManager修改功能
    std::cout << std::endl;
    std::cout << "=======验证OrderManager修改功能=======" << std::endl;
    bool ret3 = order_manager.update_order_status(order1.get_id(),
                                                  OrderStatus::CANCELLED);
    if (ret3) {
        std::cout << "订单修改成功！修改后内容：" << std::endl;
        std::cout << "订单编号："
                  << order_manager.get_order_content(order1.get_id())->get_id()
                  << "  ";
        std::cout << "订单状态："
                  << order_status_to_string(
                         order_manager.get_order_content(order1.get_id())
                             ->get_status())
                  << std::endl;
    } else {
        std::cout << "订单修改失败！查无此单！" << std::endl;
    }

    bool ret4 = order_manager.update_order_status(123, OrderStatus::CANCELLED);
    if (ret4) {
        std::cout << "订单修改成功！修改后内容：" << std::endl;
        std::cout << "订单编号："
                  << order_manager.get_order_content(order1.get_id())->get_id()
                  << "  ";
        std::cout << "订单状态："
                  << order_status_to_string(
                         order_manager.get_order_content(order1.get_id())
                             ->get_status())
                  << std::endl;
    } else {
        std::cout << "订单修改失败！查无此单！" << std::endl;
    }

    // 验证OrderBookt功能
    std::cout << std::endl;
    std::cout << "=======验证OrderBook“添加”和“打印”功能=======" << std::endl;
    OrderBook order_book;
    for (const Order& ord : orders) {
        order_book.add_order(ord);
    }
    order_book.print_book();

    std::cout << std::endl;
    std::cout << "=======验证OrderBook“撮合”功能=======" << std::endl;
    std::cout << "↓" << std::endl;
    std::cout << "=======买单和卖盘刚好吃完情况：" << std::endl;
    Order order7(128459023748920353, OrderDirection::Buy, 101,
                 10);  // 构建买单吃卖盘（刚好吃完情况）
    int ret5 = order_book.match(order7);  // 承接match后返回值
    std::cout << "撮合后新单余量：" << ret5 << std::endl;
    order_book.print_book();  // 打印看最新orderbook

    std::cout << "↓" << std::endl;
    std::cout << "=======买单吃不完卖盘情况：" << std::endl;
    Order order8(128459023748920354, OrderDirection::Buy, 101,
                 10);  // 构建买单吃卖盘（吃不完情况）
    int ret6 = order_book.match(order8);  // 承接match后返回值
    std::cout << "撮合后新单余量：" << ret6 << std::endl;
    order_book.print_book();  // 打印看最新orderbook

    std::cout << "↓" << std::endl;
    std::cout << "=======买单连吃多档、全部成交情况：" << std::endl;
    Order order9(128459023748920355, OrderDirection::Buy, 101,
                 20);  // 构建买单吃卖盘（连吃多档、全部成交情况）
    int ret7 = order_book.match(order9);  // 承接match后返回值
    std::cout << "撮合后新单余量：" << ret7 << std::endl;
    order_book.print_book();  // 打印看最新orderbook

    std::cout << "↓" << std::endl;
    std::cout << "=======买单完全吃不了卖盘情况：" << std::endl;
    Order order10(128459023748920356, OrderDirection::Buy, 100,
                  5);  // 构建买单吃卖盘（完全不吃直接挂回情况）
    int ret8 = order_book.match(order10);  // 承接match后返回值
    std::cout << "撮合后新单余量：" << ret8 << std::endl;
    order_book.print_book();  // 打印看最新orderbook

    std::cout << "↓" << std::endl;
    std::cout << "=======买单把卖盘完全吃完还有余情况：" << std::endl;
    Order order11(128459023748920357, OrderDirection::Buy, 101,
                  100);  // 构建买单吃卖盘（吃完还有余情况）
    int ret9 = order_book.match(order11);  // 承接match后返回值
    std::cout << "撮合后新单余量：" << ret9 << std::endl;
    order_book.print_book();  // 打印看最新orderbook


    return 0;
}