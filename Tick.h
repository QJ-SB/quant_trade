// Tick.h
#pragma once

class Tick {
public:
    Tick(double price, int volume);
    double get_price() const;
    int get_volume() const;

private:
    double m_price;  // 成交价格
    int m_volume;    // 成交量
};