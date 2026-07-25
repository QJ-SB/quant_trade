#include "Feed.h"

#include <cstddef>
#include <stdexcept>
#include <utility>

Feed::Feed(std::vector<Tick> ticks, TickHandler handler)
    : m_ticks(std::move(ticks)), m_handler(std::move(handler)) {
    // 时间倒退会破坏行情事件顺序；相同 timestamp 合法。
    for (std::size_t i = 1; i < m_ticks.size(); ++i) {
        if (m_ticks[i].get_timestamp() < m_ticks[i - 1].get_timestamp()) {
            throw std::runtime_error(
                "Feed ticks must be sorted by non-decreasing timestamp");
        }
    }
}

void Feed::run() {
    for (const auto& tick : m_ticks) {
        m_handler(tick);
    }
}
