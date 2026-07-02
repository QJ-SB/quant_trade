// csv_reader.h
#pragma once

#include <string>
#include <vector>

#include "Tick.h"

std::vector<Tick> read_ticks_from_csv(const std::string& path);
