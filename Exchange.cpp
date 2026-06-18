#include "Exchange.h"

#include <cassert>  //assert()
#include <vector>

#include "Order.h"  //Fill结构体

std::vector<Fill> Exchange::submit_order(const Order& order) {
    m_order_manager.add_order(order);  //添加新订单进OMS
    std::vector<Fill> fills = m_order_book.match(
        order);  //在order book中撮合新订单（并拿到撮合后fills）

    for (const auto& fill : fills) {  //循环取出fills更新OMS
        [[maybe_unused]] bool ok =
            m_order_manager.apply_fill(fill.taker_id,
                                       fill.quantity);  //更新OMS中taker
        assert(ok);                                     //断言检查返回bool

        ok = m_order_manager.apply_fill(fill.maker_id,
                                        fill.quantity);  //更新OMS中maker
        assert(ok);                                      //再次断言检查
    }

    return fills;
}

const OrderManager& Exchange::get_order_manager() const {
    return m_order_manager;
}

const OrderBook& Exchange::get_order_book() const {
    return m_order_book;
}

bool Exchange::cancel_order(uint64_t id) {
    bool ok1 = m_order_book.cancel(id);  //接收orderbook是否cancel成功
    if (ok1) {  // book撤成功，OMS同步设置状态
        [[maybe_unused]] bool ok2 =
            m_order_manager.update_order_status(id, OrderStatus::CANCELLED);
        assert(ok2);  //断言检查返回bool
    }
    return ok1;  // book没撤成,OMS不动，状态同步
}
