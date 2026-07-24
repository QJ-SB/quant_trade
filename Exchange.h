#pragma once
#include <optional>
#include <vector>

#include "Order.h"  //Fill结构体
#include "OrderBook.h"
#include "OrderManager.h"

class Exchange {
public:
    /// Exchange 是订单提交的协调入口：先登记到 OrderManager，重复 id 直接拒绝
    /// 接受后交给 OrderBook 撮合，并根据 fills 同步 OMS 状态
    /// @return std::nullopt 表示订单被拒绝；有值且 vector
    /// 为空表示已接受但本次未成交；
    ///         有值且 vector 非空表示已接受并产生了成交记录
    std::optional<std::vector<Fill>> submit_order(const Order& order);

    /// 返回内部 OrderManager 的 const 引用，用于只读观察订单状态
    /// 调用方不能通过该接口修改 OMS，返回引用而非副本
    const OrderManager& get_order_manager() const;

    /// 返回内部 OrderBook 的 const 引用，用于只读观察或调用 const 接口
    /// 调用方不能绕过 Exchange 直接挂单、撮合或撤单，返回引用而非副本
    const OrderBook& get_order_book() const;

    /// 先从 OrderBook 撤单，只有 Book 撤单成功才将 OMS 状态同步为 CANCELLED
    /// @return OrderBook 的撤单结果；false 表示订单不在 Book 中，OMS 保持不变；
    ///         true 时 OMS 随后同步为 CANCELLED
    bool cancel_order(uint64_t id);

private:
    OrderManager m_order_manager;
    OrderBook m_order_book;
};
