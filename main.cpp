#include <iostream>
#include <vector>

class Tick{
public:
    Tick(double price, int volume) : m_price(price), m_volume(volume){}

    double get_price() const {return m_price;}

    int get_volume() const {return m_volume;}

private:
    double m_price; // 成交价格
    int m_volume; // 成交量
};


int main(){

    Tick tick1(100.5, 50);
    Tick tick2(100.6, 51);
    Tick tick3(100.7, 52);

    std::vector<Tick> ticks;

    ticks.push_back(tick1);
    ticks.push_back(tick2);
    ticks.push_back(tick3);

    for(Tick& tick : ticks){
        std::cout<<"价格："<<tick.get_price()<<"  ";
        std::cout<<"成交量："<<tick.get_volume()<<std::endl;
    }

    return 0;
}