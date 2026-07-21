#include "Feed.h"

#include <cstddef>  // std::size_t
#include <stdexcept>
#include <utility>  // std::move()

Feed::Feed(std::vector<Tick> ticks, TickHandler handler)
    : m_ticks(std::move(ticks)), m_handler(std::move(handler)) {
    // 校验 m_ticks 时间戳非递减（"后一个" >= "前一个"），乱序则抛异常
    for (std::size_t i = 1; i < m_ticks.size(); ++i) {
        if (m_ticks[i].get_timestamp() < m_ticks[i - 1].get_timestamp()) {
            throw std::runtime_error(
                "Feed ticks must be sorted by non-decreasing timestamp");
        }
    }
}

void Feed::run() {
    for (const auto& tick : m_ticks) {
        m_handler(tick);  // 按逐笔数据遍历，推送给回调处理函数
    }
}
