#include <gtest/gtest.h>

#include "OrderFactory.h"

// 买入意图应正确转为 PENDING 状态的正式订单，各字段逐项传递
TEST(OrderFactoryTest, CreatesOrderFromIntent) {
    OrderFactory factory(1000);

    Order order =
        factory.create_order(OrderIntent{OrderDirection::Buy, 99.5, 7});

    EXPECT_EQ(order.get_id(), 1000u);
    EXPECT_EQ(order.get_direction(), OrderDirection::Buy);
    EXPECT_DOUBLE_EQ(order.get_price(), 99.5);
    EXPECT_EQ(order.get_quantity(), 7);
    EXPECT_EQ(order.get_status(), OrderStatus::PENDING);
}

// 工厂持有递增ID状态，连续创建应分配连续编号
TEST(OrderFactoryTest, AssignsSequentialIds) {
    OrderFactory factory(500);

    Order order1 =
        factory.create_order(OrderIntent{OrderDirection::Buy, 100.0, 5});
    Order order2 =
        factory.create_order(OrderIntent{OrderDirection::Sell, 200.0, 3});

    EXPECT_EQ(order1.get_id(), 500u);
    EXPECT_EQ(order2.get_id(), 501u);
}

// 卖出意图字段应完整传递，避免方向硬编码为买入
TEST(OrderFactoryTest, PreservesSellIntentFields) {
    OrderFactory factory(42);

    Order order =
        factory.create_order(OrderIntent{OrderDirection::Sell, 105.0, 8});

    EXPECT_EQ(order.get_id(), 42u);
    EXPECT_EQ(order.get_direction(), OrderDirection::Sell);
    EXPECT_DOUBLE_EQ(order.get_price(), 105.0);
    EXPECT_EQ(order.get_quantity(), 8);
    EXPECT_EQ(order.get_status(), OrderStatus::PENDING);
}
