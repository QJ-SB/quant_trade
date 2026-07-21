#include "ThresholdStrategy.h"

ThresholdStrategy::ThresholdStrategy(double buy_below, double sell_above,
                                     int quantity)
    : m_buy_below(buy_below), m_sell_above(sell_above),
      m_fixed_trade_qty(quantity) {}

std::optional<OrderIntent> ThresholdStrategy::on_tick(const Tick& tick) const {
    double price = tick.get_price();

    if (price < m_buy_below) {
        return OrderIntent{OrderDirection::Buy, price, m_fixed_trade_qty};
    }

    if (price > m_sell_above) {
        return OrderIntent{OrderDirection::Sell, price, m_fixed_trade_qty};
    }

    return std::nullopt;
}
