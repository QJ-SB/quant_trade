#pragma once

#include <string>
#include <vector>

#include "Tick.h"

/// 读取无表头 CSV，每行格式为 timestamp,price,volume。
/// 按文件顺序返回 Tick，不负责排序或时间顺序校验。
std::vector<Tick> read_ticks_from_csv(const std::string& path);
