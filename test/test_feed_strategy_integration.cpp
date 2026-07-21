#include <gtest/gtest.h>
#include <vector>

#include "Feed.h"
#include "OrderIntent.h"
#include "ThresholdStrategy.h"

// 验证 Feed 推送 Tick → 策略判断 → 收集 OrderIntent 的完整链路
TEST(FeedStrategyIntegrationTest, FeedDrivesStrategyAndCollectsOrderIntents) {
    // 构造测试数据：3 个 Tick，仅后两个触发交易
    std::vector<Tick> ticks{
        Tick(1000, 101.0, 10),  // 区间内，不交易
        Tick(1001, 99.0, 10),   // 低于买入阈值 100.0，触发 Buy
        Tick(1002, 106.0, 10)   // 高于卖出阈值 105.0，触发 Sell
    };

    ThresholdStrategy strategy(100.0, 105.0, 7);
    std::vector<OrderIntent> intents;

    Feed feed(std::move(ticks), [&strategy, &intents](const Tick& tick) {
        auto intent = strategy.on_tick(tick);
        if (intent.has_value()) {
            intents.push_back(*intent);
        }
    });

    feed.run();

    ASSERT_EQ(intents.size(), 2u);

    EXPECT_EQ(intents[0].direction, OrderDirection::Buy);
    EXPECT_DOUBLE_EQ(intents[0].price, 99.0);
    EXPECT_EQ(intents[0].quantity, 7);

    EXPECT_EQ(intents[1].direction, OrderDirection::Sell);
    EXPECT_DOUBLE_EQ(intents[1].price, 106.0);
    EXPECT_EQ(intents[1].quantity, 7);
}
