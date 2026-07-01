// Feed.h
#pragma once
#include <functional>  //std::function<>
#include <vector>

#include "Tick.h"

using TickHandler = std::function<void(const Tick&)>;  // 重命名

class Feed {  // Feed管道（数据 --→ 回调处理）
public:
    Feed(std::vector<Tick> ticks,
         TickHandler handler);  // 构造Feed（逐笔成交组，逐笔行情回调函数句柄）
    void run();                 // 按时间遍历推送ticks

private:
    std::vector<Tick> m_ticks;  // 保存行情数据，内部使用
    TickHandler m_handler;      // 保存回调函数句柄，内部使用
};
