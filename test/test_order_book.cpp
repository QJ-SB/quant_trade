#include <gtest/gtest.h>

#include "OrderBook.h"


//测试match()：taker和maker交叉全成，两边离场
TEST(OrderBookTest, FullCrossFillsBothAndClearsBook) {
    OrderBook ob;
    // inline构造买盘
    ob.add_order(Order(1, OrderDirection::Buy, 100.0, 50));  //先挂买盘maker

    // match()构造对手盘
    auto fills = ob.match(Order(2, OrderDirection::Sell, 99.0,
                                50));  //让卖单taker略低，最后100.0成交，全吃完
    ASSERT_EQ(fills.size(),
              1u);  // vector.size()返回size_t，值为1【刹车位-防UB】
    EXPECT_EQ(fills.front().maker_id, 1u);  // maker是uint64_t，值为1
    EXPECT_EQ(fills.front().taker_id, 2u);  // taker是uint64_t，值为2
    EXPECT_EQ(fills.front().price,
              100.0);  //不用DOUBLE_EQ，因为没做浮点运算，强相等，值为100.0
    EXPECT_EQ(fills.front().quantity, 50);  // quantity-int，值为50

    EXPECT_FALSE(ob.cancel(1));  //探针验证状态（已不在本地索引m_order_index）
    EXPECT_FALSE(ob.cancel(2));
}

//测试match()：taker吃maker余20，并用流动性探针吃完后检测留存量和状态
TEST(OrderBookTest, TakerLargerEatsMakerAndRequeuesRemainder) {
    OrderBook ob;
    // inline构造买盘
    ob.add_order(Order(1, OrderDirection::Buy, 100.0, 30));  //买盘maker

    // match()构造对手盘
    auto fills1 = ob.match(
        Order(2, OrderDirection::Sell, 99.0, 50));  //卖单taker50，吃完maker剩20
    ASSERT_EQ(fills1.size(),
              1u);  // vector.size()返回size_t，值为1【刹车位-防UB】
    EXPECT_EQ(fills1.front().maker_id, 1u);  // maker是uint64_t，值为1
    EXPECT_EQ(fills1.front().taker_id, 2u);  // taker是uint64_t，值为2
    EXPECT_EQ(fills1.front().price,
              100.0);  //不用DOUBLE_EQ，因为没做浮点运算，强相等，值为100.0
    EXPECT_EQ(fills1.front().quantity, 30);  // quantity-int，值为30
    EXPECT_FALSE(ob.cancel(1));  //探针验证状态（已不在本地索引m_order_index）

    //构建流动性探针，反向单吃完上轮回挂（taker → maker），同时验证“量+状态”
    auto fills2 = ob.match(Order(3, OrderDirection::Buy, 99.0,
                                 20));  //买单taker20，刚好吃完上轮余量
    ASSERT_EQ(fills2.size(),
              1u);  // vector.size()返回size_t，值为1【刹车位-防UB】
    //头一次验证match后taker余量返挂后身份转变maker
    EXPECT_EQ(fills2.front().maker_id, 2u);  // maker是uint64_t，值为2
    EXPECT_EQ(fills2.front().taker_id, 3u);  // taker是uint64_t，值为3
    EXPECT_EQ(fills2.front().price,
              99.0);  //不用DOUBLE_EQ，因为没做浮点运算，强相等，值为99.0
    EXPECT_EQ(fills2.front().quantity, 20);  // quantity-int，值为20
    EXPECT_FALSE(ob.cancel(2));  //探针验证状态（已不在本地索引m_order_index）
    EXPECT_FALSE(ob.cancel(3));  //探针单也消失
}

//测试match()：maker被taker吃余20，并用流动性探针吃完后检测留存量和状态
TEST(OrderBookTest, TakerSmallerFullyFilledMakerKeepsRemainder) {
    OrderBook ob;
    // inline构造买盘
    ob.add_order(Order(1, OrderDirection::Buy, 100.0, 50));  //买盘maker

    // match()构造对手盘
    auto fills1 = ob.match(
        Order(2, OrderDirection::Sell, 99.0, 30));  //卖单taker30，吃掉maker30
    ASSERT_EQ(fills1.size(),
              1u);  // vector.size()返回size_t，值为1【刹车位-防UB】
    EXPECT_EQ(fills1.front().maker_id, 1u);  // maker是uint64_t，值为1
    EXPECT_EQ(fills1.front().taker_id, 2u);  // taker是uint64_t，值为2
    EXPECT_EQ(fills1.front().price,
              100.0);  //不用DOUBLE_EQ，因为没做浮点运算，强相等，值为100.0
    EXPECT_EQ(fills1.front().quantity, 30);  // quantity-int，值为30
    EXPECT_FALSE(ob.cancel(2));  //探针验证状态（已不在本地索引m_order_index）

    //构建流动性探针，反向单吃完上轮剩余maker，同时验证“量+状态”
    auto fills2 = ob.match(Order(3, OrderDirection::Sell, 100.0,
                                 20));  //卖单taker20，刚好吃完上轮余量
    ASSERT_EQ(fills2.size(),
              1u);  // vector.size()返回size_t，值为1【刹车位-防UB】
    EXPECT_EQ(fills2.front().maker_id, 1u);  // maker是uint64_t，值为1
    EXPECT_EQ(fills2.front().taker_id, 3u);  // taker是uint64_t，值为3
    EXPECT_EQ(fills2.front().price,
              100.0);  //不用DOUBLE_EQ，因为没做浮点运算，强相等，值为100.0
    EXPECT_EQ(fills2.front().quantity, 20);  // quantity-int，值为20
    EXPECT_FALSE(ob.cancel(1));  //探针验证状态（已不在本地索引m_order_index）
    EXPECT_FALSE(ob.cancel(3));  //探针单也消失
}

//测试match()：taker和maker完全不交叉，直接挂簿
TEST(OrderBookTest, NonCrossingTakerRestsNoFill) {
    OrderBook ob;
    // inline构造买盘
    ob.add_order(Order(1, OrderDirection::Buy, 100.0, 50));  //买盘maker

    // match()构造对手盘
    auto fills1 = ob.match(
        Order(2, OrderDirection::Sell, 101.0, 50));  //卖单taker完全吃不了买盘
    EXPECT_TRUE(
        fills1
            .empty());  // vector<Fill>空，没成交【不用ASSERT-因为后续不解引用】
    EXPECT_TRUE(ob.cancel(2));  //验证taker挂进去了
    EXPECT_TRUE(ob.cancel(1));  //验证maker还在
}

//测试match()：买单吃卖盘（验证maker里的价格priority）
TEST(OrderBookTest, BuyOrder_EatsTwoSellLevels_LowerPriceGetsPriority) {
    OrderBook ob;
    // inline构造卖盘
    ob.add_order(Order(1, OrderDirection::Sell, 101.0, 50));  //卖盘maker-先挂
    ob.add_order(Order(2, OrderDirection::Sell, 100.0, 50));  //后挂

    // match()构造对手盘
    auto fills1 = ob.match(
        Order(3, OrderDirection::Buy, 102.0, 50));  //买单taker吃完一级卖盘
    ASSERT_EQ(fills1.size(),
              1u);  // vector.size()返回size_t，值为1【刹车位-防UB】
    EXPECT_EQ(fills1.front().maker_id, 2u);  // maker是uint64_t，值为2
    EXPECT_EQ(fills1.front().taker_id, 3u);  // taker是uint64_t，值为3
    EXPECT_EQ(fills1.front().price,
              100.0);  //不用DOUBLE_EQ，因为没做浮点运算，强相等，值为100.0
    EXPECT_EQ(fills1.front().quantity, 50);  // quantity-int，值为50

    // cancel探针检测
    EXPECT_TRUE(ob.cancel(1));  //订单1还在
}

//测试match()：买单吃卖盘（验证maker里的时间priority）
TEST(OrderBookTest,
     BuyOrder_EatsTwoSellOrders_SamePrice_EarlierOrderGetsPriority) {
    OrderBook ob;
    // inline构造卖盘
    ob.add_order(Order(1, OrderDirection::Sell, 100.0, 50));  //卖盘maker-先挂
    ob.add_order(Order(2, OrderDirection::Sell, 100.0, 50));  //后挂（同价）

    // match()构造对手盘
    auto fills1 = ob.match(
        Order(3, OrderDirection::Buy, 101.0, 50));  //买单taker吃完一级卖盘
    ASSERT_EQ(fills1.size(),
              1u);  // vector.size()返回size_t，值为1【刹车位-防UB】
    EXPECT_EQ(fills1.front().maker_id, 1u);  // maker是uint64_t，值为1
    EXPECT_EQ(fills1.front().taker_id, 3u);  // taker是uint64_t，值为3
    EXPECT_EQ(fills1.front().price,
              100.0);  //不用DOUBLE_EQ，因为没做浮点运算，强相等，值为100.0
    EXPECT_EQ(fills1.front().quantity, 50);  // quantity-int，值为50

    // cancel探针检测
    EXPECT_TRUE(ob.cancel(2));  //订单2还在
}

//【镜像】
//测试match()：卖单吃买盘（验证maker里的价格priority）
TEST(OrderBookTest, SellOrder_EatsTwoBuyLevels_HigherPriceGetsPriority) {
    OrderBook ob;
    // inline构造买盘
    ob.add_order(Order(1, OrderDirection::Buy, 99.0, 50));  //买盘maker-先挂
    ob.add_order(Order(2, OrderDirection::Buy, 100.0, 50));  //后挂

    // match()构造对手盘
    auto fills1 = ob.match(
        Order(3, OrderDirection::Sell, 98.0, 50));  //卖单taker吃完一级买盘
    ASSERT_EQ(fills1.size(),
              1u);  // vector.size()返回size_t，值为1【刹车位-防UB】
    EXPECT_EQ(fills1.front().maker_id, 2u);  // maker是uint64_t，值为2
    EXPECT_EQ(fills1.front().taker_id, 3u);  // taker是uint64_t，值为3
    EXPECT_EQ(fills1.front().price,
              100.0);  //不用DOUBLE_EQ，因为没做浮点运算，强相等，值为100.0
    EXPECT_EQ(fills1.front().quantity, 50);  // quantity-int，值为50

    // cancel探针检测
    EXPECT_TRUE(ob.cancel(1));  //订单1还在
}

//【镜像】
//测试match()：卖单吃买盘（验证maker里的时间priority）
TEST(OrderBookTest,
     SellOrder_EatsTwoBuyOrders_SamePrice_EarlierOrderGetsPriority) {
    OrderBook ob;
    // inline构造买盘
    ob.add_order(Order(1, OrderDirection::Buy, 100.0, 50));  //卖盘maker-先挂
    ob.add_order(Order(2, OrderDirection::Buy, 100.0, 50));  //后挂（同价）

    // match()构造对手盘
    auto fills1 = ob.match(
        Order(3, OrderDirection::Sell, 99.0, 50));  //卖单taker吃完一级买盘
    ASSERT_EQ(fills1.size(),
              1u);  // vector.size()返回size_t，值为1【刹车位-防UB】
    EXPECT_EQ(fills1.front().maker_id, 1u);  // maker是uint64_t，值为1
    EXPECT_EQ(fills1.front().taker_id, 3u);  // taker是uint64_t，值为3
    EXPECT_EQ(fills1.front().price,
              100.0);  //不用DOUBLE_EQ，因为没做浮点运算，强相等，值为100.0
    EXPECT_EQ(fills1.front().quantity, 50);  // quantity-int，值为50

    // cancel探针检测
    EXPECT_TRUE(ob.cancel(2));  //订单2还在
}