#include <gtest/gtest.h>
#include <optional>  //std::optional
#include <utility>   // std::move()
#include <vector>

#include "Exchange.h"
#include "Feed.h"
#include "Order.h"
#include "OrderFactory.h"
#include "ThresholdStrategy.h"

// 功能描述：验证完整执行链路 Feed→策略→订单工厂→交易所→OMS 的全流程集成
// 参数说明：(无)
// 返回值说明：(无)
// 异常说明：(无)

// submit_order 接口三态返回契约：
// 态1 - 有效 optional + 空 fills：订单被受理，无对手盘可撮合，挂入订单簿
// 态2 - 有效 optional + 非空
// fills：订单被受理，与订单簿内对手盘发生撮合，产生成交记录 态3 -
// nullopt：订单被拒绝（重复 id），不入簿、不撮合
//
// 本次撮合成交价规则：taker 限价单成交价取 maker 挂单价（97.0），
// 而非 taker 自身委托价（98.0），由 OrderBook::match_against 内部实现决定

TEST(FeedStrategyExchangeIntegrationTest,
     GeneratedBuyOrderExecutesAgainstRestingSell) {
    // ========== Arrange ==========

    // 初始化交易所
    Exchange ex;

    // 初始化策略：buy_below=100, sell_above=105, quantity=7
    ThresholdStrategy strategy(100.0, 105.0, 7);

    // 初始化订单工厂：first_id=1000
    OrderFactory factory(1000);

    // 预埋 maker 卖单：id=500, Sell, price=97.0, quantity=7
    Order maker_order(500, OrderDirection::Sell, 97.0, 7);
    auto maker_result = ex.submit_order(maker_order);
    // 校验：正常单必受理
    ASSERT_TRUE(maker_result.has_value());
    // 态1：无对手盘，fills 为空，订单挂入订单簿
    EXPECT_TRUE(maker_result->empty());

    // 构造两段 Tick 行情数据
    // Tick 1: 价格 101，区间内不触发
    // Tick 2: 价格 98，低于买入阈值 100，触发 Buy 意图
    std::vector<Tick> ticks{Tick(1000, 101.0, 10), Tick(1001, 98.0, 10)};

    // 定义外部变量存储回调结果（lambda 内不堆积断言，统一后置校验）
    std::optional<std::vector<Fill>> strategy_submit_result;
    int submitted_count = 0;

    // ========== Act ==========

    // Feed 回调链路：tick→on_tick→生成意向→创建订单→提交订单→保存结果
    Feed feed(std::move(ticks), [&](const Tick& tick) {
        auto intent = strategy.on_tick(tick);             // 策略判断
        if (intent.has_value()) {                         // 若触发交易
            Order order = factory.create_order(*intent);  // 意图→正式订单
            auto result = ex.submit_order(order);         // 提交到交易所
            strategy_submit_result = std::move(result);  // 保存提交结果
            ++submitted_count;                           // 计数
        }
    });

    feed.run();

    // ========== Assert ==========

    // --- 第1层：订单提交计数校验 ---
    // 仅 Tick 2（价格 98）触发一次 Buy 意图，应产生 1 笔订单提交
    EXPECT_EQ(submitted_count, 1);

    // 校验策略提交结果：正常单必受理
    ASSERT_TRUE(strategy_submit_result.has_value());
    // --- 第2层：Fill 成交明细校验 ---
    // 态2：Buy@98 与预埋 Sell@97 撮合，成交价取 maker 挂单价 97
    ASSERT_EQ(strategy_submit_result->size(), 1u);
    const Fill& fill = strategy_submit_result->front();
    EXPECT_EQ(fill.taker_id, 1000u);
    EXPECT_EQ(fill.maker_id, 500u);
    EXPECT_EQ(fill.quantity, 7);
    EXPECT_DOUBLE_EQ(fill.price, 97.0);

    // --- 第3层：OMS 订单状态校验 ---
    const OrderManager& om = ex.get_order_manager();
    EXPECT_EQ(om.get_order_size(), 2u);

    // taker 订单 id=1000：已完全成交，状态 FILLED，剩余量 0
    auto order1000 = om.get_order_content(1000);
    ASSERT_TRUE(order1000.has_value());
    EXPECT_EQ(order1000->get_status(), OrderStatus::FILLED);
    EXPECT_EQ(order1000->get_quantity(), 0);

    // maker 订单 id=500：已完全成交，状态 FILLED，剩余量 0
    auto order500 = om.get_order_content(500);
    ASSERT_TRUE(order500.has_value());
    EXPECT_EQ(order500->get_status(), OrderStatus::FILLED);
    EXPECT_EQ(order500->get_quantity(), 0);

    // --- 第4层：盘口撤单收尾校验 ---
    // 完全成交订单已不在订单簿中，撤销应返回 false
    EXPECT_FALSE(ex.cancel_order(1000));
    EXPECT_FALSE(ex.cancel_order(500));
}
