#pragma once
#include <functional>
#include <utility>
#include <vector>

#include "Tick.h"

using TickHandler = std::function<void(const Tick&)>;

/// 按输入顺序回放 Tick，并将每笔行情推送给 handler。
class Feed {
public:
    /// 保存 ticks 与 handler【 sink parameter下沉参数-“值传递 + move” 】
    /// 要求 timestamp 非递减，Feed不自动排序 ↓
    /// （相同 timestamp 合法，若时间倒退则抛出 std::runtime_error）
    ///
    /// @param ticks 行情快照集
    /// @param handler 回调函数
    Feed(std::vector<Tick> ticks, TickHandler handler);

    /// 按保存顺序逐笔调用 handler，不模拟真实时间间隔。
    void run();

private:
    std::vector<Tick> m_ticks;
    TickHandler m_handler;
};
