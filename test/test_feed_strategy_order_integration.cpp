#include <gtest/gtest.h>
#include <vector>

#include "Feed.h"
#include "Order.h"
#include "OrderFactory.h"
#include "ThresholdStrategy.h"

// 验证端到端链路：Feed 推送 Tick → 策略生成 OrderIntent → 工厂生成正式 Order
TEST(FeedStrategyOrderIntegrationTest, FeedDrivesStrategyAndCreatesOrders) {
    std::vector<Tick> ticks{
        Tick(1000, 101.0, 10),  // 区间内，不交易
        Tick(1001, 99.0, 10),   // 低于买入阈值 100.0，触发 Buy
        Tick(1002, 106.0, 10)   // 高于卖出阈值 105.0，触发 Sell
    };

    ThresholdStrategy strategy(100.0, 105.0, 7);
    OrderFactory factory(1000);
    std::vector<Order> orders;

    Feed feed(std::move(ticks), [&](const Tick& tick) {
        auto intent = strategy.on_tick(tick);
        if (intent.has_value()) {
            orders.push_back(factory.create_order(*intent));
        }
    });

    feed.run();

    ASSERT_EQ(orders.size(), 2u);

    EXPECT_EQ(orders[0].get_id(), 1000u);
    EXPECT_EQ(orders[0].get_direction(), OrderDirection::Buy);
    EXPECT_DOUBLE_EQ(orders[0].get_price(), 99.0);
    EXPECT_EQ(orders[0].get_quantity(), 7);
    EXPECT_EQ(orders[0].get_status(), OrderStatus::PENDING);

    EXPECT_EQ(orders[1].get_id(), 1001u);
    EXPECT_EQ(orders[1].get_direction(), OrderDirection::Sell);
    EXPECT_DOUBLE_EQ(orders[1].get_price(), 106.0);
    EXPECT_EQ(orders[1].get_quantity(), 7);
    EXPECT_EQ(orders[1].get_status(), OrderStatus::PENDING);
}
