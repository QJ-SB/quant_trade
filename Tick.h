// Tick.h
#pragma once

class Tick {
public:
    Tick(double price, int volume);  //构造函数

    double get_price() const;     //成交价格 getter
    int get_volume() const;       //成交量 getter
    double get_turnover() const;  //成交额 getter

private:
    double m_price;  //成交价格
    int m_volume;    //成交量
};