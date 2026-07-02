#【版本】
cmake_minimum_required(VERSION 3.14)    # 要求的最低 CMake 版本
project(quant_trade)                    # 项目名（跟仓库名一致）


#【C++标准】
set(CMAKE_CXX_STANDARD 17)              # 用 C++17（= g++ 的 -std=c++17）
set(CMAKE_CXX_STANDARD_REQUIRED ON)     # 强制 C++17，不许静默降级


#【CMake配置导出】
# 开启导出编译命令数据库 compile_commands.json
# 生成文件位于 build/compile_commands.json
# 作用：给Clangd、VSCode C/C++插件、Clang-Tidy等代码分析工具读取
# 工具通过该文件获取每个cpp的编译参数、头文件路径、宏定义，实现代码补全、静态检查、跳转无报错
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


#【ASan块】
# Sanitizer 开关：日常 OFF；测试内存/UB 时 `cmake -DENABLE_ASAN=ON ..` 打开
# 注意：编译和链接都要挂 sanitizer，否则链接报 undefined reference
option(ENABLE_ASAN "Build with Address + UB Sanitizers" OFF)
if(ENABLE_ASAN)
    add_compile_options(-fsanitize=address,undefined -g -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
endif()


#【库/可执行】
# ---
# 1. 把业务核心代码打包成静态库（quant_core）
add_library(quant_core
            Tick.cpp
            Order.cpp
            OrderManager.cpp
            OrderBook.cpp
            Exchange.cpp
            Feed.cpp
            csv_reader.cpp)
# ---
# 2. 设置库的头文件搜索目录
# CMAKE_CURRENT_SOURCE_DIR：当前 CMakeLists.txt 所在的源码根目录
# PUBLIC 含义：当前库自身编译需要该头文件路径，同时所有链接 quant_core 的程序（如trade、单元测试）自动继承该包含路径，无需重复配置
target_include_directories(quant_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
# ---
# 3. 主程序只保留入口main.cpp
add_executable(trade main.cpp)
# ---
# 4. 主程序链接依赖核心库
target_link_libraries(trade PRIVATE quant_core)


# 【FetchContent】
# ---
# 引入CMake内置的第三方资源拉取模块FetchContent
# 作用：编译阶段自动从Git仓库下载、构建第三方依赖，无需用户本地手动安装gtest
include(FetchContent)
# ---
# 声明要拉取的第三方依赖：GoogleTest单元测试框架
# 参数1：依赖内部标识名googletest，后续使用该名称引用
# 参数2：指定gtest的官方Git仓库地址
# 参数3：钉死版本；要更新自己去 gtest releases 核
FetchContent_Declare(googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0) 
# ---
# 执行拉取+编译流程：
# 1. 不存在源码则自动克隆对应tag的gtest代码
# 2. 自动执行gtest自身CMake构建，生成GTest::gtest / GTest::gtest_main库目标
# 3. 把编译好的库暴露给当前项目，供单元测试程序链接使用
# 最低CMake版本要求3.14，当前项目版本3.22完全兼容无问题         
FetchContent_MakeAvailable(googletest)


#【测试】
# ---
# 开启CMake内置测试系统，启用ctest命令批量执行测试用例
enable_testing()
# ---
# 编译生成专门运行单元测试的可执行程序 unit_tests
# 仅传入测试源码文件 test/test_smoke.cpp，存放各类GTest测试逻辑
add_executable(unit_tests
            test/test_smoke.cpp
            test/test_order_manager.cpp
            test/test_order_book.cpp
            test/test_exchange.cpp
            test/test_feed.cpp
            test/test_csv_reader.cpp)
# ---
# 为测试程序配置依赖链接
# PRIVATE：仅unit_tests内部使用这些依赖，不会传递给其他上层目标
# 1. quant_core：项目核心业务库，测试代码需要调用订单、撮合等业务类
# 2. GTest::gtest_main：GTest提供内置main函数，测试文件无需手写程序入口
target_link_libraries(unit_tests PRIVATE quant_core GTest::gtest_main)
# ---
# 导入CMake内置的GoogleTest配套工具模块，提供gtest_discover_tests功能
include(GoogleTest)
# ---
# 自动扫描unit_tests程序内所有TEST()测试用例，注册到ctest测试系统
# 执行ctest命令时，会自动遍历、运行全部测试并输出通过/失败结果
gtest_discover_tests(unit_tests)
