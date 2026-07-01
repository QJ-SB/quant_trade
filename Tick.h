// Tick.h
#pragma once

class Tick {
public:
    Tick(long long timestamp, double price,
         int volume);  // 构造Tick（时间戳，价格，成交量）

    double get_price() const;         // 成交价格 getter
    int get_volume() const;           // 成交量 getter
    double get_turnover() const;      // 成交额 getter
    long long get_timestamp() const;  // 时间戳getter

private:
    long long m_timestamp;  // 时间戳-epoch milliseconds
    double m_price;         // 成交价格
    int m_volume;           // 成交量
};
