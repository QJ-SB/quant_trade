#include <iostream>
#include <vector>

#include "Exchange.h"      //交易所
#include "Order.h"         //订单
#include "OrderBook.h"     //订单簿
#include "OrderManager.h"  //订单管理
#include "Tick.h"          //逐笔行情


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

    // 验证封装好manager和book的接口层Exchange
    std::cout << std::endl;
    std::cout << "=======验证Exchange=======" << std::endl;
    std::cout << "-------买卖刚好全部成交" << std::endl;
    Exchange exchange1;                                          //建交易所
    const auto& order_manager1 = exchange1.get_order_manager();  //拿引用
    const auto& order_book1 = exchange1.get_order_book();        //拿引用
    Order order1(1001, OrderDirection::Buy, 167.0, 100);         //创订单
    Order order2(1002, OrderDirection::Sell, 167.0, 100);
    auto fills1 = exchange1.submit_order(order1);  // submit订单
    std::cout << "【成交信息】" << std::endl;
    for (const auto& fill : fills1) {
        std::cout << "taker_id:" << fill.taker_id;
        std::cout << " maker_id:" << fill.maker_id;
        std::cout << " price:" << fill.price;
        std::cout << " quantity:" << fill.quantity << std::endl;
    }
    order_manager1.print_order();  //打印看看
    order_book1.print_book();
    auto fills2 = exchange1.submit_order(order2);  // submit对手盘
    std::cout << "【成交信息】" << std::endl;
    for (const auto& fill : fills2) {
        std::cout << "taker_id:" << fill.taker_id;
        std::cout << " maker_id:" << fill.maker_id;
        std::cout << " price:" << fill.price;
        std::cout << " quantity:" << fill.quantity << std::endl;
    }
    order_manager1.print_order();  //再打印
    order_book1.print_book();


    std::cout << "-------部分成交" << std::endl;
    Exchange exchange2;                                          //建交易所
    const auto& order_manager2 = exchange2.get_order_manager();  //拿引用
    const auto& order_book2 = exchange2.get_order_book();        //拿引用
    Order order3(2001, OrderDirection::Sell, 167.0, 80);         //创订单
    Order order4(2002, OrderDirection::Sell, 167.5, 70);
    Order order5(2003, OrderDirection::Buy, 168.0, 100);
    Order order8(2004, OrderDirection::Buy, 170.2, 60);
    exchange2.submit_order(order3);  // submit订单
    exchange2.submit_order(order4);
    order_manager2.print_order();  //打印看看
    order_book2.print_book();
    auto fills3 = exchange2.submit_order(order5);  // submit对手盘
    std::cout << "【成交信息】" << std::endl;
    for (const auto& fill : fills3) {
        std::cout << "taker_id:" << fill.taker_id;
        std::cout << " maker_id:" << fill.maker_id;
        std::cout << " price:" << fill.price;
        std::cout << " quantity:" << fill.quantity << std::endl;
    }
    order_manager2.print_order();  //再打印
    order_book2.print_book();
    auto fills5 = exchange2.submit_order(order8);  //全吃完再挂回
    std::cout << "【成交信息】" << std::endl;
    for (const auto& fill : fills5) {
        std::cout << "taker_id:" << fill.taker_id;
        std::cout << " maker_id:" << fill.maker_id;
        std::cout << " price:" << fill.price;
        std::cout << " quantity:" << fill.quantity << std::endl;
    }
    order_manager2.print_order();  //再打印
    order_book2.print_book();


    std::cout << "-------完全不成交" << std::endl;
    Exchange exchange3;                                          //建交易所
    const auto& order_manager3 = exchange3.get_order_manager();  //拿引用
    const auto& order_book3 = exchange3.get_order_book();        //拿引用
    Order order6(3001, OrderDirection::Sell, 167.1, 80);         //创订单
    Order order7(3002, OrderDirection::Buy, 161.2, 70);
    exchange3.submit_order(order6);  // submit订单
    order_manager3.print_order();    //打印看看
    order_book3.print_book();
    auto fills4 = exchange3.submit_order(order7);  // submit对手盘
    std::cout << "【成交信息】" << std::endl;
    for (const auto& fill : fills4) {
        std::cout << "taker_id:" << fill.taker_id;
        std::cout << " maker_id:" << fill.maker_id;
        std::cout << " price:" << fill.price;
        std::cout << " quantity:" << fill.quantity << std::endl;
    }
    order_manager3.print_order();  //再打印
    order_book3.print_book();


    return 0;
}