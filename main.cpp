#include "Tick.h"
#include <iostream>
#include <vector>


int main() {

    Tick tick1(100.5, 50);
    Tick tick2(100.6, 51);
    Tick tick3(100.7, 52);

    std::vector<Tick> ticks;

    ticks.push_back(tick1);
    ticks.push_back(tick2);
    ticks.push_back(tick3);

    for (const Tick& tick : ticks) {
        std::cout << "价格：" << tick.get_price() << "  ";
        std::cout << "成交量：" << tick.get_volume() << "  ";
        std::cout << "成交额：" << tick.get_turnover() << std::endl;
    }

    return 0;
}