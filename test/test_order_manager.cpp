#include <gtest/gtest.h>

#include "OrderManager.h"

// 测试夹具：每个测试用例都会新建一个 OrderManager 并预先插入一个基础订单
class OrderManagerTest : public testing::Test {
protected:
    OrderManager om;

    void SetUp() override {
        // 使用 ASSERT_TRUE 保证前置条件满足，否则测试立即停止
        ASSERT_TRUE(om.add_order(Order(1001, OrderDirection::Buy, 167.0, 100)));
    }
};

// 测试add_order()：验证 seed 进去的单能原样读回
TEST_F(OrderManagerTest, AddThenGetReturnsSameOrder) {
    // 此时 om 已有一个 ID=1001 的订单，验证 size 为 1（这是测试的一部分）
    EXPECT_EQ(om.get_order_size(), 1u);

    auto fetched = om.get_order_content(1001);  // 拿到std::optional<Order>拷贝
    ASSERT_TRUE(fetched.has_value());  // 解前刹车，否则下面解空optional = UB
    EXPECT_EQ(fetched->get_id(), 1001u);                       // 验证id
    EXPECT_EQ(fetched->get_status(), OrderStatus::PENDING);    // 验证状态
    EXPECT_EQ(fetched->get_direction(), OrderDirection::Buy);  // 验证方向
    EXPECT_DOUBLE_EQ(fetched->get_price(), 167.0);  // 验证价格（用 DOUBLE_EQ）
    EXPECT_EQ(fetched->get_quantity(), 100);        // 验证数量
}

// 测试add_order()：添加重复id订单
TEST_F(OrderManagerTest, AddSameIdOrderReturnFalse) {
    EXPECT_EQ(om.get_order_size(), 1u);  // 验证初始 size

    // 尝试添加重复 ID，应返回 false
    EXPECT_FALSE(om.add_order(Order(1001, OrderDirection::Buy, 167.0, 100)));
    EXPECT_EQ(om.get_order_size(), 1u);  // 验证原单没被覆盖
}

// 测试get_order_content()：获取不存在id的订单信息
TEST_F(OrderManagerTest, GetNonExistentOrderIdReturnNullopt) {
    auto fetched = om.get_order_content(9999);  // 拿std::optional<Order>拷贝
    ASSERT_FALSE(fetched.has_value());  // 拿到的optional应该是空（nullopt）
}

// 测试apply_fill()：状态机|部分成交
TEST_F(OrderManagerTest, ApplyFillPartialSetsPartiallyFilled) {
    EXPECT_TRUE(om.apply_fill(1001, 30));       // 部分吃，吃30
    auto fetched = om.get_order_content(1001);  // 取回
    ASSERT_TRUE(fetched.has_value());           // 有值（刹车检测）
    EXPECT_EQ(fetched->get_quantity(), 70);     // 量
    EXPECT_EQ(fetched->get_status(), OrderStatus::PARTIALLY_FILLED);  // 状态
}

// 测试apply_fill()：状态机|部分成交 → 全部成交
TEST_F(OrderManagerTest, ApplyFillSetsStatusCumulativeToFilled) {
    EXPECT_TRUE(om.apply_fill(1001, 30));       // 部分吃，吃30
    EXPECT_TRUE(om.apply_fill(1001, 70));       // 再吃，吃70，吃完
    auto fetched = om.get_order_content(1001);  // 取回
    ASSERT_TRUE(fetched.has_value());           // 有值（刹车检测）
    EXPECT_EQ(fetched->get_quantity(), 0);      // 量
    EXPECT_EQ(fetched->get_status(), OrderStatus::FILLED);  // 状态
}

// 测试apply_fill()：一次全部成交
TEST_F(OrderManagerTest, ApplyFillSetsDirectlyToFilled) {
    EXPECT_TRUE(om.apply_fill(1001, 100));      // 一次全吃100
    auto fetched = om.get_order_content(1001);  // 取回
    ASSERT_TRUE(fetched.has_value());           // 有值（刹车检测）
    EXPECT_EQ(fetched->get_quantity(), 0);      // 量
    EXPECT_EQ(fetched->get_status(), OrderStatus::FILLED);  // 状态
}

// 测试apply_fill()：对不存在id执行
TEST_F(OrderManagerTest, ApplyFillNonExistentIdReturnFalse) {
    EXPECT_FALSE(om.apply_fill(9999, 100));  // apply_fill()不存在id
}

// 测试apply_fill()：传入amount > 存在quantity（仅Debug模式）
// 如果没有定义“NDEBUG”宏 → debug版本 → apply_fill()内部assert正常
// 如果定义了“NDEBUG”宏 → release版本 → apply_fill()内部assert被删除
// 仅Debug编译，Release直接移除该测试 ↓
#ifndef NDEBUG
TEST_F(OrderManagerTest, ApplyFillExcessAborts) {
    EXPECT_DEATH(om.apply_fill(1001, 999), "Assertion");
}
#endif

// 测试update_order_status()：update正常id
TEST_F(OrderManagerTest, UpdateStatusToCancelled) {
    EXPECT_TRUE(om.update_order_status(1001, OrderStatus::CANCELLED));
    auto fetched = om.get_order_content(1001);
    ASSERT_TRUE(fetched.has_value());
    EXPECT_EQ(fetched->get_quantity(), 100);
    EXPECT_EQ(fetched->get_status(), OrderStatus::CANCELLED);
}

// 测试update_order_status()：update不存在id
TEST_F(OrderManagerTest, UpdateNonExistentOrderIdReturnFalse) {
    EXPECT_FALSE(om.update_order_status(9999, OrderStatus::CANCELLED));
}