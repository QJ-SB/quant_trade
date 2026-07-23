#include <gtest/gtest.h>
#include <limits>  // 提供 quiet_NaN() / infinity() 用于构造非法价格测试数据

#include "PreTradeRisk.h"

// 测试夹具：固定配置上限 max_order_quantity=100, max_order_notional=10000.0
class PreTradeRiskTest : public ::testing::Test {
protected:
    PreTradeRisk risk{100, 10000.0};
};

// 【价格测试】
// 价格=0 为非法价格，应在第一道校验被拦截
TEST_F(PreTradeRiskTest, RejectsZeroPrice) {
    OrderIntent intent{OrderDirection::Buy, 0.0, 10};
    EXPECT_EQ(risk.check(intent), RiskDecision::InvalidPrice);
}

// 价格=-1.0 为非法价格（负数），应在第一道校验被拦截
TEST_F(PreTradeRiskTest, RejectsNegativePrice) {
    OrderIntent intent{OrderDirection::Buy, -1.0, 10};
    EXPECT_EQ(risk.check(intent), RiskDecision::InvalidPrice);
}

// 价格=NaN / 正无穷 / 负无穷 均为非有限值，统一在第一道校验被 isfinite 拦截
TEST_F(PreTradeRiskTest, RejectsNonFinitePrice) {
    OrderIntent nan_intent{OrderDirection::Buy,
                           std::numeric_limits<double>::quiet_NaN(), 10};
    EXPECT_EQ(risk.check(nan_intent), RiskDecision::InvalidPrice);

    OrderIntent inf_intent{OrderDirection::Buy,
                           std::numeric_limits<double>::infinity(), 10};
    EXPECT_EQ(risk.check(inf_intent), RiskDecision::InvalidPrice);

    OrderIntent neg_inf_intent{OrderDirection::Buy,
                               -std::numeric_limits<double>::infinity(), 10};
    EXPECT_EQ(risk.check(neg_inf_intent), RiskDecision::InvalidPrice);
}

//【数量测试】
// 数量=0 为非正数，应在第二道校验被拦截
TEST_F(PreTradeRiskTest, RejectsZeroQuantity) {
    OrderIntent intent{OrderDirection::Buy, 10.0, 0};
    EXPECT_EQ(risk.check(intent), RiskDecision::NonPositiveQuantity);
}

// 数量=-1 为非正数，应在第二道校验被拦截
TEST_F(PreTradeRiskTest, RejectsNegativeQuantity) {
    OrderIntent intent{OrderDirection::Buy, 10.0, -1};
    EXPECT_EQ(risk.check(intent), RiskDecision::NonPositiveQuantity);
}

// 数量=101 超出单笔上限 100，应在第三道校验被拦截
TEST_F(PreTradeRiskTest, RejectsQuantityAboveLimit) {
    OrderIntent intent{OrderDirection::Buy, 10.0, 101};
    EXPECT_EQ(risk.check(intent), RiskDecision::QuantityLimitExceeded);
}

// 【名义金额测试】
// 价格=50, 数量=50，名义金额=2500，全部在限制区间内
TEST_F(PreTradeRiskTest, AcceptsIntentWithinLimits) {
    OrderIntent intent{OrderDirection::Buy, 50.0, 50};
    EXPECT_EQ(risk.check(intent), RiskDecision::Accepted);
}

// 价格=100, 数量=100，名义金额=10000，刚好等于上限边界
TEST_F(PreTradeRiskTest, AcceptsIntentExactlyAtLimits) {
    OrderIntent intent{OrderDirection::Buy, 100.0, 100};
    EXPECT_EQ(risk.check(intent), RiskDecision::Accepted);
}

// 价格=101, 数量=100，名义金额=10100 超出上限 10000，应在第四道校验被拦截
TEST_F(PreTradeRiskTest, RejectsNotionalLimitExceeded) {
    OrderIntent intent{OrderDirection::Buy, 101.0, 100};
    EXPECT_EQ(risk.check(intent), RiskDecision::NotionalLimitExceeded);
}
