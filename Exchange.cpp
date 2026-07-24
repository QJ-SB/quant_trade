#include "Exchange.h"

#include <cassert>
#include <vector>

#include "Order.h"

std::optional<std::vector<Fill>> Exchange::submit_order(const Order& order) {
    // OrderManager::add_order() 是进入撮合前的去重闸门
    // 重复 id 必须在调用 OrderBook::match() 前拒绝，否则重复订单可能进入 book
    if (!m_order_manager.add_order(order))
        return std::nullopt;

    std::vector<Fill> fills = m_order_book.match(order);

    // match() 返回的是成交事实，每条 fill 需同时应用到 taker 和 maker 的 OMS
    // 记录 每笔 fill 的 quantity 对 taker 和 maker 相同，两侧使用同一个
    // fill.quantity
    for (const auto& fill : fills) {
        // apply_fill() 必须在 assert 外调用，避免 NDEBUG 删除实际状态更新
        [[maybe_unused]] bool ok =
            m_order_manager.apply_fill(fill.taker_id, fill.quantity);
        assert(ok);  // 内部不变量守卫：撮合产生的 id 不应在 OMS 中缺失

        ok = m_order_manager.apply_fill(fill.maker_id, fill.quantity);
        assert(ok);
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
    // OrderBook 是订单是否仍可撤的物理真相源，先撤 Book
    bool ok1 = m_order_book.cancel(id);
    if (ok1) {
        // 只有 Book 撤单成功才将 OMS 状态更新为 CANCELLED
        // update_order_status() 必须在 assert 外调用，避免 NDEBUG 删除副作用
        [[maybe_unused]] bool ok2 =
            m_order_manager.update_order_status(id, OrderStatus::CANCELLED);
        assert(ok2);  // 内部不变量守卫：Book 与 OMS 状态不应脱节
    }
    return ok1;
}
