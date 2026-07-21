// 阈值策略：价格跌破买入阈值生成买单意图，涨破卖出阈值生成卖单意图
#pragma once

#include <optional>

#include "OrderIntent.h"
#include "Tick.h"

class ThresholdStrategy {
public:
    ThresholdStrategy(double buy_below, double sell_above, int quantity);

    std::optional<OrderIntent> on_tick(const Tick& tick) const;

private:
    double m_buy_below;     // 买入阈值
    double m_sell_above;    // 卖出阈值
    int m_fixed_trade_qty;  // 固定交易数量
};
