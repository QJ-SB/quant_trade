#pragma once

#include <cstdint>

/// 记录一次撮合成交；quantity 同时作用于 taker 与 maker，
/// price 取 maker 的挂单价格。
struct Fill {
    uint64_t taker_id;
    uint64_t maker_id;
    int quantity;
    double price;
};
