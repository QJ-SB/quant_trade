#include <gtest/gtest.h>

#include "Exchange.h"


// 测试夹具：每个用例独立 Exchange 实例
class ExchangeTest : public testing::Test {
protected:
    Exchange ex;
};

// 关切1：买单全吃卖单（full cross）
// 摆盘：先挂 sell 10@100，再挂 buy 10@100，后者完全吃掉前者
TEST_F(ExchangeTest, FullCrossBuyTakerFillsSellMaker) {
    // 1. 提交卖单（无对手盘，应挂起，返回空 fills）
    auto fills1 = ex.submit_order(Order(1, OrderDirection::Sell, 100.0, 10));
    EXPECT_TRUE(fills1.empty());  // 未成交

    // 2. 提交买单（价格匹配，数量相等，全吃）
    auto fills2 = ex.submit_order(Order(2, OrderDirection::Buy, 100.0, 10));
    // 验证返回的 fills（成交明细）
    ASSERT_EQ(fills2.size(), 1u);  // 1份成交
    EXPECT_EQ(fills2.front().taker_id, 2u);
    EXPECT_EQ(fills2.front().maker_id, 1u);
    EXPECT_EQ(fills2.front().quantity, 10);
    EXPECT_DOUBLE_EQ(fills2.front().price, 100.0);

    // 验证 OMS 状态：两张单都应变为 FILLED，剩余量 0
    const OrderManager& om = ex.get_order_manager();
    EXPECT_EQ(om.get_order_size(), 2u);
    auto order1 = om.get_order_content(1);
    ASSERT_TRUE(order1.has_value());
    EXPECT_EQ(order1->get_status(), OrderStatus::FILLED);
    EXPECT_EQ(order1->get_quantity(), 0);
    auto order2 = om.get_order_content(2);
    ASSERT_TRUE(order2.has_value());
    EXPECT_EQ(order2->get_status(), OrderStatus::FILLED);
    EXPECT_EQ(order2->get_quantity(), 0);

    // 验证盘口已空：撤销两张单应均返回 false（表示订单已不在簿中）
    EXPECT_FALSE(ex.cancel_order(1));
    EXPECT_FALSE(ex.cancel_order(2));
}

// 关切2：买单部分成交（taker 小于 maker）
// 摆盘：先挂 sell 10@100，再挂 buy 4@100，后者只吃4手，卖单剩余6手
TEST_F(ExchangeTest, PartialFillBuyTakerPartiallyFillsSellMaker) {
    auto fills1 = ex.submit_order(Order(1, OrderDirection::Sell, 100.0, 10));
    EXPECT_TRUE(fills1.empty());

    auto fills2 = ex.submit_order(Order(2, OrderDirection::Buy, 100.0, 4));
    ASSERT_EQ(fills2.size(), 1u);
    EXPECT_EQ(fills2.front().taker_id, 2u);
    EXPECT_EQ(fills2.front().maker_id, 1u);
    EXPECT_EQ(fills2.front().quantity, 4);
    EXPECT_DOUBLE_EQ(fills2.front().price, 100.0);

    const OrderManager& om = ex.get_order_manager();
    EXPECT_EQ(om.get_order_size(), 2u);

    // 买单全成交 -> FILLED, qty 0
    auto order2 = om.get_order_content(2);
    ASSERT_TRUE(order2.has_value());
    EXPECT_EQ(order2->get_status(), OrderStatus::FILLED);
    EXPECT_EQ(order2->get_quantity(), 0);

    // 卖单部分成交 -> PARTIALLY_FILLED, qty 6
    auto order1 = om.get_order_content(1);
    ASSERT_TRUE(order1.has_value());
    EXPECT_EQ(order1->get_status(), OrderStatus::PARTIALLY_FILLED);
    EXPECT_EQ(order1->get_quantity(), 6);

    // 探针：卖单残量仍在盘上，可撤；买单已离场，不可撤
    EXPECT_TRUE(ex.cancel_order(1));  // 撤掉残量
    EXPECT_FALSE(ex.cancel_order(2));
}