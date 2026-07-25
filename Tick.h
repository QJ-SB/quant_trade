#pragma once

/// Tick 表示行情管道中的一笔逐笔成交。
class Tick {
public:
    Tick(long long timestamp, double price, int volume);

    double get_price() const;
    int get_volume() const;

    /// 成交额按 price * volume 即时计算，不存储冗余状态。
    double get_turnover() const;

    long long get_timestamp() const;

private:
    long long m_timestamp;  // Unix epoch 毫秒
    double m_price;
    int m_volume;
};
