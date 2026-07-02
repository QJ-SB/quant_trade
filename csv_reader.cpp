// csv_reader.cpp
#include "csv_reader.h"

#include <fstream>    // std::ifstream类
#include <sstream>    // std::stringstream类
#include <stdexcept>  // std::runtime_error(), std::exception
#include <string>     // std::getline() 自动转为std::string

std::vector<Tick> read_ticks_from_csv(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + path);
    }

    std::vector<Tick> ticks;
    std::string line;
    long long line_no = 0;

    while (std::getline(file, line)) {
        ++line_no;

        std::vector<std::string> fields;
        std::stringstream ss(line);
        std::string field;
        while (std::getline(ss, field, ',')) {
            fields.push_back(field);
        }

        if (fields.size() != 3) {
            throw std::runtime_error("Bad row at line " +
                                     std::to_string(line_no) + ": " + line);
        }

        long long timestamp;  //外部声明（让try内部纯净捕捉三转换异常）
        double price;
        int volume;
        try {
            timestamp = std::stoll(fields[0]);  // long long
            price = std::stod(fields[1]);       // double
            volume = std::stoi(fields[2]);      // int
        } catch (const std::exception&) {
            throw std::runtime_error("Bad row at line " +
                                     std::to_string(line_no) + ": " + line);
        }
        ticks.emplace_back(timestamp, price, volume);  //推入外部vecto
    }

    return ticks;
}
