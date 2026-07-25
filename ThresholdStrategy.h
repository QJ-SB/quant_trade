#pragma once

#include <optional>

#include "OrderIntent.h"
#include "Tick.h"

/// 根据成交价格与阈值生成固定数量的买卖意图。
class ThresholdStrategy {
public:
    ThresholdStrategy(double buy_below, double sell_above, int quantity);

    /// price < buy_below 时生成 Buy，price > sell_above 时生成 Sell；
    /// 位于阈值之间或恰好等于任一阈值时返回 std::nullopt。
    std::optional<OrderIntent> on_tick(const Tick& tick) const;

private:
    double m_buy_below;
    double m_sell_above;
    int m_fixed_trade_qty;
};
