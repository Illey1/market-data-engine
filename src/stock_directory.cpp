#include "market_data_engine/stock_directory.hpp"

#include "big_endian.hpp"

#include <stdexcept>

namespace market_data_engine {
namespace {

std::string read_alpha(std::span<const std::uint8_t> bytes) {
    std::string value(bytes.begin(), bytes.end());
    while (!value.empty() && value.back() == ' ') {
        value.pop_back();
    }
    return value;
}

}

StockDirectoryMessage parse_stock_directory(std::span<const std::uint8_t> bytes) {
    if (bytes.size() != 39) {
        throw std::invalid_argument("Stock Directory payload must be exactly 39 bytes");
    }
    if (bytes[0] != 'R') {
        throw std::invalid_argument("Stock Directory message type must be 'R'");
    }

    return StockDirectoryMessage{
        detail::read_be16(bytes.subspan<1, 2>()),
        detail::read_be16(bytes.subspan<3, 2>()),
        detail::read_be48(bytes.subspan<5, 6>()),
        read_alpha(bytes.subspan<11, 8>()),
        static_cast<char>(bytes[19]),
        static_cast<char>(bytes[20]),
        detail::read_be32(bytes.subspan<21, 4>()),
        static_cast<char>(bytes[25]),
        static_cast<char>(bytes[26]),
        read_alpha(bytes.subspan<27, 2>()),
        static_cast<char>(bytes[29]),
        static_cast<char>(bytes[30]),
        static_cast<char>(bytes[31]),
        static_cast<char>(bytes[32]),
        static_cast<char>(bytes[33]),
        detail::read_be32(bytes.subspan<34, 4>()),
        static_cast<char>(bytes[38]),
    };
}

}
