#include "market_data_engine/add_order.hpp"

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

AddOrderMessage parse_add_order(std::span<const std::uint8_t> bytes) {
    if (bytes.size() != 36) {
        throw std::invalid_argument("Add Order payload must be exactly 36 bytes");
    }
    if (bytes[0] != 'A') {
        throw std::invalid_argument("Add Order message type must be 'A'");
    }

    return AddOrderMessage{
        detail::read_be16(bytes.subspan<1, 2>()),
        detail::read_be16(bytes.subspan<3, 2>()),
        detail::read_be48(bytes.subspan<5, 6>()),
        detail::read_be64(bytes.subspan<11, 8>()),
        static_cast<char>(bytes[19]),
        detail::read_be32(bytes.subspan<20, 4>()),
        read_alpha(bytes.subspan<24, 8>()),
        detail::read_be32(bytes.subspan<32, 4>()),
    };
}

AddOrderWithMpidMessage parse_add_order_with_mpid(std::span<const std::uint8_t> bytes) {
    if (bytes.size() != 40) {
        throw std::invalid_argument("Add Order with MPID payload must be exactly 40 bytes");
    }
    if (bytes[0] != 'F') {
        throw std::invalid_argument("Add Order with MPID message type must be 'F'");
    }

    return AddOrderWithMpidMessage{
        detail::read_be16(bytes.subspan<1, 2>()),
        detail::read_be16(bytes.subspan<3, 2>()),
        detail::read_be48(bytes.subspan<5, 6>()),
        detail::read_be64(bytes.subspan<11, 8>()),
        static_cast<char>(bytes[19]),
        detail::read_be32(bytes.subspan<20, 4>()),
        read_alpha(bytes.subspan<24, 8>()),
        detail::read_be32(bytes.subspan<32, 4>()),
        read_alpha(bytes.subspan<36, 4>()),
    };
}

}
