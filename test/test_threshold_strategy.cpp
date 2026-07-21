#include <gtest/gtest.h>

#include "ThresholdStrategy.h"

// 价格跌破买入阈值，应生成买入意图
TEST(ThresholdStrategyTest, ReturnsBuyIntentWhenPriceBelowBuyThreshold) {
    ThresholdStrategy strat(100.0, 200.0, 10);

    auto result = strat.on_tick(Tick(0, 99.5, 100));

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->direction, OrderDirection::Buy);
    EXPECT_DOUBLE_EQ(result->price, 99.5);
    EXPECT_EQ(result->quantity, 10);
}

// 价格涨破卖出阈值，应生成卖出意图
TEST(ThresholdStrategyTest, ReturnsSellIntentWhenPriceAboveSellThreshold) {
    ThresholdStrategy strat(100.0, 200.0, 10);

    auto result = strat.on_tick(Tick(0, 200.5, 100));

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->direction, OrderDirection::Sell);
    EXPECT_DOUBLE_EQ(result->price, 200.5);
    EXPECT_EQ(result->quantity, 10);
}

// 价格在买卖阈值之间，不生成任何意图
TEST(ThresholdStrategyTest, ReturnsNulloptWhenPriceBetweenThresholds) {
    ThresholdStrategy strat(100.0, 200.0, 10);

    auto result = strat.on_tick(Tick(0, 150.0, 100));

    EXPECT_FALSE(result.has_value());
}

// 价格等于买入阈值（严格比较），不生成意图
TEST(ThresholdStrategyTest, ReturnsNulloptWhenPriceEqualsBuyThreshold) {
    ThresholdStrategy strat(100.0, 200.0, 10);

    auto result = strat.on_tick(Tick(0, 100.0, 100));

    EXPECT_FALSE(result.has_value());
}

// 价格等于卖出阈值（严格比较），不生成意图
TEST(ThresholdStrategyTest, ReturnsNulloptWhenPriceEqualsSellThreshold) {
    ThresholdStrategy strat(100.0, 200.0, 10);

    auto result = strat.on_tick(Tick(0, 200.0, 100));

    EXPECT_FALSE(result.has_value());
}
