#pragma once

#include <cstdint>
#include <iosfwd>
#include <vector>

namespace market_data_engine {

bool read_binary_file_message(std::istream& input,
                              std::vector<std::uint8_t>& payload);

}
