#include "csv_reader.h"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

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

        // 仅将字段转换异常统一为坏行错误；Tick 构造保持在 try 外，
        // 避免把与输入格式无关的异常误报为 Bad row。
        long long timestamp;
        double price;
        int volume;
        try {
            timestamp = std::stoll(fields[0]);
            price = std::stod(fields[1]);
            volume = std::stoi(fields[2]);
        } catch (const std::exception&) {
            throw std::runtime_error("Bad row at line " +
                                     std::to_string(line_no) + ": " + line);
        }

        ticks.emplace_back(timestamp, price, volume);
    }

    return ticks;
}
