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
    auto r1 = ex.submit_order(Order(1, OrderDirection::Sell, 100.0, 10));
    ASSERT_TRUE(r1.has_value());  // 正常单必受理
    EXPECT_TRUE(r1->empty());     // 未成交

    // 2. 提交买单（价格匹配，数量相等，全吃）
    auto r2 = ex.submit_order(Order(2, OrderDirection::Buy, 100.0, 10));
    ASSERT_TRUE(r2.has_value());  // 正常单必受理
    // 验证返回的 fills（成交明细）
    ASSERT_EQ(r2->size(), 1u);  // 1份成交
    EXPECT_EQ(r2->front().taker_id, 2u);
    EXPECT_EQ(r2->front().maker_id, 1u);
    EXPECT_EQ(r2->front().quantity, 10);
    EXPECT_DOUBLE_EQ(r2->front().price, 100.0);

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
    auto r1 = ex.submit_order(Order(1, OrderDirection::Sell, 100.0, 10));
    ASSERT_TRUE(r1.has_value());
    EXPECT_TRUE(r1->empty());

    auto r2 = ex.submit_order(Order(2, OrderDirection::Buy, 100.0, 4));
    ASSERT_TRUE(r2.has_value());
    ASSERT_EQ(r2->size(), 1u);
    EXPECT_EQ(r2->front().taker_id, 2u);
    EXPECT_EQ(r2->front().maker_id, 1u);
    EXPECT_EQ(r2->front().quantity, 4);
    EXPECT_DOUBLE_EQ(r2->front().price, 100.0);

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

// 关切3：taker 残量挂回翻成 maker，随后被另一张单吃掉（角色翻转）
// 摆盘：sell 5@100 挂上 -> buy 8@100 来吃，吃5剩3，残量挂回买盘 -> sell 3@100
// 来吃残量
TEST_F(ExchangeTest, TakerRemainderBecomesMakerThenEaten) {
    // 第1步：挂 sell 5@100（maker A）
    auto r1 = ex.submit_order(Order(1, OrderDirection::Sell, 100.0, 5));
    ASSERT_TRUE(r1.has_value());
    EXPECT_TRUE(r1->empty());

    // 第2步：buy 8@100 来吃，吃光 A 的5，剩余3挂回买盘
    auto r2 = ex.submit_order(Order(2, OrderDirection::Buy, 100.0, 8));
    ASSERT_TRUE(r2.has_value());
    ASSERT_EQ(r2->size(), 1u);
    EXPECT_EQ(r2->front().taker_id, 2u);
    EXPECT_EQ(r2->front().maker_id, 1u);
    EXPECT_EQ(r2->front().quantity, 5);
    EXPECT_DOUBLE_EQ(r2->front().price, 100.0);

    // 第3步：sell 3@100 来吃，应该吃掉 id=2 的残量 3
    auto r3 = ex.submit_order(Order(3, OrderDirection::Sell, 100.0, 3));
    ASSERT_TRUE(r3.has_value());
    ASSERT_EQ(r3->size(), 1u);
    EXPECT_EQ(r3->front().taker_id, 3u);
    EXPECT_EQ(r3->front().maker_id, 2u);  // 关键：maker 是 id=2
    EXPECT_EQ(r3->front().quantity, 3);
    EXPECT_DOUBLE_EQ(r3->front().price, 100.0);

    // 验证 OMS 终态：三张单全部 FILLED，qty 0
    const OrderManager& om = ex.get_order_manager();
    EXPECT_EQ(om.get_order_size(), 3u);

    auto order1 = om.get_order_content(1);
    ASSERT_TRUE(order1.has_value());
    EXPECT_EQ(order1->get_status(), OrderStatus::FILLED);
    EXPECT_EQ(order1->get_quantity(), 0);

    auto order2 = om.get_order_content(2);
    ASSERT_TRUE(order2.has_value());
    EXPECT_EQ(order2->get_status(), OrderStatus::FILLED);
    EXPECT_EQ(order2->get_quantity(), 0);

    auto order3 = om.get_order_content(3);
    ASSERT_TRUE(order3.has_value());
    EXPECT_EQ(order3->get_status(), OrderStatus::FILLED);
    EXPECT_EQ(order3->get_quantity(), 0);

    // 探针：所有订单已成交离场，撤销均返回 false
    EXPECT_FALSE(ex.cancel_order(1));
    EXPECT_FALSE(ex.cancel_order(2));
    EXPECT_FALSE(ex.cancel_order(3));
}

// 关切4：重复 id 提交应被拒，不得在 book 留下孤儿幽灵节点
TEST_F(ExchangeTest, DuplicateIdSubmitRejectedNoGhostLiquidity) {
    // ① 挂一张不交叉的卖单 id=7，建立节点 A
    auto r1 = ex.submit_order(Order(7, OrderDirection::Sell, 100.0, 10));
    ASSERT_TRUE(r1.has_value());
    EXPECT_TRUE(r1->empty());

    // ② 同一 id=7 重复提交：应被拒，不进 book
    //   现在 optional 使拒单直接可观测，升级为真断言
    auto r2 = ex.submit_order(Order(7, OrderDirection::Sell, 100.0, 10));
    EXPECT_FALSE(r2.has_value());  // 【新】拒单直接可观测！

    // ③ 撤 id=7。fixed 删唯一节点盘口空；buggy 删 B 留孤儿 A
    ASSERT_TRUE(ex.cancel_order(7));  // 前提守卫：必须撤得掉

    // ④ taker 吃 100 价位，把孤儿现形
    auto r4 = ex.submit_order(Order(8, OrderDirection::Buy, 100.0, 10));
    ASSERT_TRUE(r4.has_value());  // 正常单必受理
    // 【核心判别】fixed 盘口空 → 0 条成交；buggy 孤儿 A 存在 → 1 条成交
    EXPECT_EQ(r4->size(), 0u);

    // 强化判别：fixed 下 taker(8) 吃不到东西挂回买盘，残量仍 10
    //   （buggy 下 8 吃了 A → qty 0，这条同样翻红，与上互证）
    const OrderManager& om = ex.get_order_manager();
    auto order8 = om.get_order_content(8);
    ASSERT_TRUE(order8.has_value());
    EXPECT_EQ(order8->get_quantity(), 10);
}