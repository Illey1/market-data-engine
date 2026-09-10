#pragma once

#include <cstdint>
#include <iosfwd>
#include <vector>

namespace market_data_engine {

enum class BinaryFileReadResult {
    message,
    end_of_session,
    end_of_file,
};

BinaryFileReadResult read_binary_file_message(
    std::istream& input, std::vector<std::uint8_t>& payload);

}
