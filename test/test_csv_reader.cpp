#include <cstdio>  //std::remove() 来自C库的接口
#include <fstream>
#include <gtest/gtest.h>
#include <string>  //std::string.c_str() 转c类型字符串const char*

#include "csv_reader.h"

//正常读取测试
TEST(CsvReaderTest, ReadsValidRowsInOrder) {
    const std::string path = "test_csv_reader_valid.csv";
    {
        std::ofstream out(path);
        out << "1768407000000,100.5,30\n";
        out << "1768407000001,101.5,40\n";
        out << "1768407000002,102.5,50\n";
    }

    std::vector<Tick> ticks = read_ticks_from_csv(path);

    ASSERT_EQ(ticks.size(), 3u);
    EXPECT_EQ(ticks[0].get_timestamp(), 1768407000000LL);
    EXPECT_DOUBLE_EQ(ticks[0].get_price(), 100.5);
    EXPECT_EQ(ticks[0].get_volume(), 30);
    EXPECT_EQ(ticks[1].get_timestamp(), 1768407000001LL);
    EXPECT_EQ(ticks[2].get_timestamp(), 1768407000002LL);

    std::remove(path.c_str());
}

//坏行测试：数量error
TEST(CsvReaderTest, ThrowsOnWrongFieldCount) {
    const std::string path = "test_csv_reader_wrong_count.csv";
    {
        std::ofstream out(path);
        out << "1768407000000,100.5,30\n";
        out << "1768407000001,101.5\n";  // 结构性坏行：字段数=2
        out << "1768407000002,102.5,50\n";
    }

    EXPECT_THROW(read_ticks_from_csv(path), std::runtime_error);

    std::remove(path.c_str());  //仅接收const char*
}

//坏行测试：类型error
// 转换失败坏行测试
TEST(CsvReaderTest, ThrowsOnInvalidFieldContent) {
    const std::string path = "test_csv_reader_invalid_content.csv";
    {
        std::ofstream out(path);
        out << "1768407000000,100.5,30\n";
        out << "abc,100.5,30\n";  // 内容类坏行：首字段无法转换为数字
        out << "1768407000002,102.5,50\n";
    }

    EXPECT_THROW(read_ticks_from_csv(path), std::runtime_error);

    std::remove(path.c_str());
}
