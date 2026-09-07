#include "market_data_engine/add_order.hpp"

#include "big_endian.hpp"

#include <stdexcept>
#include <utility>

namespace market_data_engine {

AddOrderMessage parse_add_order(std::span<const std::uint8_t> bytes) {
    if (bytes.size() != 36) {
        throw std::invalid_argument("Add Order payload must be exactly 36 bytes");
    }
    if (bytes[0] != 'A') {
        throw std::invalid_argument("Add Order message type must be 'A'");
    }

    std::string stock(bytes.begin() + 24, bytes.begin() + 32);
    while (!stock.empty() && stock.back() == ' ') {
        stock.pop_back();
    }

    return AddOrderMessage{
        detail::read_be16(bytes.subspan<1, 2>()),
        detail::read_be16(bytes.subspan<3, 2>()),
        detail::read_be48(bytes.subspan<5, 6>()),
        detail::read_be64(bytes.subspan<11, 8>()),
        static_cast<char>(bytes[19]),
        detail::read_be32(bytes.subspan<20, 4>()),
        std::move(stock),
        detail::read_be32(bytes.subspan<32, 4>()),
    };
}

}
