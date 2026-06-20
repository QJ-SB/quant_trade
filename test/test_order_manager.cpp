#include <gtest/gtest.h>

#include "OrderManager.h"  // 它会带进 Order.h

// 测试add_order()：添加订单并返回相同信息
TEST(OrderManagerTest, AddThenGetReturnsSameOrder) {
    OrderManager om;
    // inline构造，防止再用外界order副本
    EXPECT_TRUE(om.add_order(Order(1001, OrderDirection::Buy, 167.0,
                                   100)));  // 验证添加后add_order返回true
    EXPECT_EQ(om.get_order_size(), 1u);     // 验证返回类型uint64_t和值

    auto fetched = om.get_order_content(1001);  // 拿到std::optional<Order>拷贝
    ASSERT_TRUE(fetched.has_value());  // 解前刹车，否则下面解空optional = UB
    EXPECT_EQ(fetched->get_id(), 1001u);                       // 验证id
    EXPECT_EQ(fetched->get_status(), OrderStatus::PENDING);    // 验证状态
    EXPECT_EQ(fetched->get_direction(), OrderDirection::Buy);  // 验证方向
    EXPECT_DOUBLE_EQ(fetched->get_price(), 167.0);  // 验证价格（用 DOUBLE_EQ）
    EXPECT_EQ(fetched->get_quantity(), 100);        // 验证数量
}

// 测试add_order()：添加重复id订单
TEST(OrderManagerTest, AddSameIdOrderReturnFalse) {
    OrderManager om;
    // inline构造，防止再用外界order副本
    EXPECT_TRUE(om.add_order(Order(1001, OrderDirection::Buy, 167.0,
                                   100)));  // 验证添加后add_order返回true
    EXPECT_EQ(om.get_order_size(), 1u);     // 验证返回类型uint64_t和值

    EXPECT_FALSE(om.add_order(
        Order(1001, OrderDirection::Buy, 167.0, 100)));  // 再add重复id
    EXPECT_EQ(om.get_order_size(), 1u);  // 验证原单没被覆盖
}

// 测试get_order_content()：获取不存在id的订单信息
TEST(OrderManagerTest, GetNonExistentOrderIdReturnNullopt) {
    OrderManager om;
    // inline构造，防止再用外界order副本
    EXPECT_TRUE(om.add_order(Order(1001, OrderDirection::Buy, 167.0,
                                   100)));  // 验证添加后add_order返回true

    auto fetched = om.get_order_content(9999);  // 拿std::optional<Order>拷贝
    ASSERT_FALSE(fetched.has_value());  // 拿到的optional应该是空（nullopt）
}

// 测试apply_fill()：状态机|部分成交
TEST(OrderManagerTest, ApplyFillPartialSetsPartiallyFilled) {
    OrderManager om;
    // inline构造，防止再用外界order副本
    EXPECT_TRUE(om.add_order(Order(1001, OrderDirection::Buy, 167.0,
                                   100)));  // 验证添加后add_order返回true

    EXPECT_TRUE(om.apply_fill(1001, 30));       // 部分吃，吃30
    auto fetched = om.get_order_content(1001);  // 取回
    ASSERT_TRUE(fetched.has_value());           // 有值（刹车检测）
    EXPECT_EQ(fetched->get_quantity(), 70);     // 量
    EXPECT_EQ(fetched->get_status(), OrderStatus::PARTIALLY_FILLED);  // 状态
}

// 测试apply_fill()：状态机|部分成交 → 全部成交
TEST(OrderManagerTest, ApplyFillSetsStatusCumulativeToFilled) {
    OrderManager om;
    // inline构造，防止再用外界order副本
    EXPECT_TRUE(om.add_order(Order(1001, OrderDirection::Buy, 167.0,
                                   100)));  // 验证添加后add_order返回true

    EXPECT_TRUE(om.apply_fill(1001, 30));       // 部分吃，吃30
    EXPECT_TRUE(om.apply_fill(1001, 70));       // 再吃，吃70，吃完
    auto fetched = om.get_order_content(1001);  // 取回
    ASSERT_TRUE(fetched.has_value());           // 有值（刹车检测）
    EXPECT_EQ(fetched->get_quantity(), 0);      // 量
    EXPECT_EQ(fetched->get_status(), OrderStatus::FILLED);  // 状态
}

// 测试apply_fill()：状态机|一次全部成交
TEST(OrderManagerTest, ApplyFillSetsDirectlyToFilled) {
    OrderManager om;
    // inline构造，防止再用外界order副本
    EXPECT_TRUE(om.add_order(Order(1001, OrderDirection::Buy, 167.0,
                                   100)));  // 验证添加后add_order返回true

    EXPECT_TRUE(om.apply_fill(1001, 100));      // 一次全吃100
    auto fetched = om.get_order_content(1001);  // 取回
    ASSERT_TRUE(fetched.has_value());           // 有值（刹车检测）
    EXPECT_EQ(fetched->get_quantity(), 0);      // 量
    EXPECT_EQ(fetched->get_status(), OrderStatus::FILLED);  // 状态
}

// 测试apply_fill()：对不存在id执行
TEST(OrderManagerTest, ApplyFillNonExistentIdReturnFalse) {
    OrderManager om;
    // inline构造，防止再用外界order副本
    EXPECT_TRUE(om.add_order(Order(1001, OrderDirection::Buy, 167.0,
                                   100)));  // 验证添加后add_order返回true

    EXPECT_FALSE(om.apply_fill(9999, 100));  // apply_fill()不存在id
}

// 测试apply_fill()：传入amount > 存在quantity
// 如果没有定义“NDEBUG”宏 → debug版本 → apply_fill()内部assert正常
// 如果定义了“NDEBUG”宏 → release版本 → apply_fill()内部assert被删除
// 仅Debug编译，Release直接移除该测试 ↓
#ifndef NDEBUG
TEST(OrderManagerTest, ApplyFillExcessAborts) {
    OrderManager om;
    // inline构造，防止再用外界order副本
    EXPECT_TRUE(om.add_order(Order(1001, OrderDirection::Buy, 167.0,
                                   100)));  // 验证添加后add_order返回true


    EXPECT_DEATH(om.apply_fill(1001, 999),
                 "Assertion");  // apply_fill()过多amount，程序abort
}
#endif

// 测试update_order_status()：update正常id
TEST(OrderManagerTest, UpdateStatusToCancelled) {
    OrderManager om;
    // inline构造，防止再用外界order副本
    EXPECT_TRUE(om.add_order(Order(1001, OrderDirection::Buy, 167.0,
                                   100)));  // 验证添加后add_order返回true

    EXPECT_TRUE(om.update_order_status(
        1001, OrderStatus::CANCELLED));         // 修改状态为Cancelled
    auto fetched = om.get_order_content(1001);  // 取回
    ASSERT_TRUE(fetched.has_value());           // 有值（刹车检测）
    EXPECT_EQ(fetched->get_quantity(), 100);    // 量
    EXPECT_EQ(fetched->get_status(), OrderStatus::CANCELLED);  // 状态
}

// 测试update_order_status()：update不存在id
TEST(OrderManagerTest, UpdateNonExistentOrderIdReturnFalse) {
    OrderManager om;
    // inline构造，防止再用外界order副本
    EXPECT_TRUE(om.add_order(Order(1001, OrderDirection::Buy, 167.0,
                                   100)));  // 验证添加后add_order返回true

    EXPECT_FALSE(om.update_order_status(
        9999, OrderStatus::CANCELLED));  // 修改状态返回false
}
