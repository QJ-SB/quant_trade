cmake_minimum_required(VERSION 3.10)    # 要求的最低 CMake 版本
project(quant_trade)                    # 项目名（跟仓库名一致）

set(CMAKE_CXX_STANDARD 17)              # 用 C++17（= g++ 的 -std=c++17）
set(CMAKE_CXX_STANDARD_REQUIRED ON)     # 强制 C++17，不许静默降级

# Sanitizer 开关：日常 OFF；测试内存/UB 时 `cmake -DENABLE_ASAN=ON ..` 打开
# 注意：编译和链接都要挂 sanitizer，否则链接报 undefined reference
option(ENABLE_ASAN "Build with Address + UB Sanitizers" OFF)
if(ENABLE_ASAN)
    add_compile_options(-fsanitize=address,undefined -g -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
endif()

# 可执行文件 trade，由这些 .cpp 编译（以后加类就往后面续文件名）
add_executable(trade
            main.cpp
            Tick.cpp
            Order.cpp
            OrderManager.cpp 
            OrderBook.cpp 
            Exchange.cpp)