cmake_minimum_required(VERSION 3.10)    # 要求的最低 CMake 版本
project(quant_trade)                    # 项目名

set(CMAKE_CXX_STANDARD 17)              # C++17(等于 g++ 的 -std=c++17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)     # 强制 C++17,不许降级

# 生成可执行文件 trade,由这几个 .cpp 编译(以后加类,就往这后面加文件名)
add_executable(trade main.cpp Tick.cpp Order.cpp OrderManager.cpp OrderBook.cpp)