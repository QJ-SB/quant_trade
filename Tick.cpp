#include "Tick.h"

Tick::Tick(double price, int volume) : m_price(price), m_volume(volume) {}

double Tick::get_price() const {
    return m_price;
}
int Tick::get_volume() const {
    return m_volume;
}

double Tick::get_turnover() const {
    return m_price * m_volume;
}
