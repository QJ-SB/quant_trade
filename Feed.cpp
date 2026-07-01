#include "Feed.h"

Feed::Feed(std::vector<Tick> ticks, TickHandler handler)
    : m_ticks(std::move(ticks)), m_handler(std::move(handler)) {
}  // 移动构造初始化

void Feed::run() {
    for (const auto& tick : m_ticks) {
        m_handler(tick);  // 按逐笔数据遍历，推送给回调处理函数
    }
}
