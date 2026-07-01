#include <gtest/gtest.h>

#include "Feed.h"

TEST(FeedTest, PushTicksInOrder) {
    std::vector<Tick> recorded;  //外部维护一份ticks拷贝，防止检测时UAF
    std::vector<Tick> v = {
        //递增构造测试ticks
        {1768407000000LL, 100.5, 30},
        {1768407000001LL, 101.5, 40},
        {1768407000002LL, 102.5, 50},
    };

    //构造管道，移动传入构造ticks，并用lambda捕获检测函数充当回调接口
    Feed f(std::move(v), [&recorded](const Tick& t) { recorded.push_back(t); });
    f.run();  //推送，外部recorded会拷贝一份m_ticks

    //检测recorded间接检测m_ticks
    EXPECT_EQ(recorded.size(), 3u);
    EXPECT_EQ(recorded[0].get_timestamp(), 1768407000000LL);
    EXPECT_EQ(recorded[1].get_timestamp(), 1768407000001LL);
    EXPECT_EQ(recorded[2].get_timestamp(), 1768407000002LL);
    EXPECT_DOUBLE_EQ(recorded[0].get_price(), 100.5);
    EXPECT_EQ(recorded[0].get_volume(), 30);
}
