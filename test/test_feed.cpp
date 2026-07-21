#include <gtest/gtest.h>

#include "Feed.h"

// 测试：传入时间戳正序（递增）的ticks序列，Feed应按顺序推送给回调函数
TEST(FeedTest, PushTicksInOrder) {
    std::vector<Tick> recorded;  // 外部维护一份ticks拷贝，防止检测时UAF
    std::vector<Tick> v = {
        // 递增构造测试ticks
        {1768407000000LL, 100.5, 30},
        {1768407000001LL, 101.5, 40},
        {1768407000002LL, 102.5, 50},
    };

    // 构造管道，移动传入构造ticks，并用lambda捕获检测函数充当回调接口
    Feed f(std::move(v), [&recorded](const Tick& t) { recorded.push_back(t); });
    f.run();  // 推送，外部recorded会拷贝一份m_ticks

    // 检测recorded间接检测m_ticks
    EXPECT_EQ(recorded.size(), 3u);
    EXPECT_EQ(recorded[0].get_timestamp(), 1768407000000LL);
    EXPECT_EQ(recorded[1].get_timestamp(), 1768407000001LL);
    EXPECT_EQ(recorded[2].get_timestamp(), 1768407000002LL);
    EXPECT_DOUBLE_EQ(recorded[0].get_price(), 100.5);
    EXPECT_EQ(recorded[0].get_volume(), 30);
}

// 测试：传入时间戳乱序（倒退）的ticks序列，构造Feed时应抛出std::runtime_error
TEST(FeedTest, ThrowsOnOutOfOrderTimestamps) {
    std::vector<Tick> v = {
        {1000, 100.5, 30},
        {999, 101.5, 40},  // 时间戳倒退，应触发异常
    };

    // 构造Feed时预期抛出 std::runtime_error
    EXPECT_THROW(Feed(std::move(v), [](const Tick&) {}), std::runtime_error);
}

// 测试：传入时间戳相等或递增的合法序列，应正常运行并按序收到所有数据
TEST(FeedTest, AllowsEqualTimestamps) {
    std::vector<Tick> recorded;
    std::vector<Tick> v = {
        {1000, 100.5, 30},
        {1000, 101.5, 40},  // 相等时间戳，应视为合法
        {1001, 102.5, 50},
    };

    Feed f(std::move(v), [&recorded](const Tick& t) { recorded.push_back(t); });
    f.run();

    // 验证按顺序收到3个时间戳
    EXPECT_EQ(recorded.size(), 3u);
    EXPECT_EQ(recorded[0].get_timestamp(), 1000);
    EXPECT_EQ(recorded[1].get_timestamp(), 1000);
    EXPECT_EQ(recorded[2].get_timestamp(), 1001);
}
