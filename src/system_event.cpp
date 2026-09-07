#include "market_data_engine/system_event.hpp"

#include "big_endian.hpp"

#include <stdexcept>

namespace market_data_engine {

SystemEventMessage parse_system_event(std::span<const std::uint8_t> bytes) {
    if (bytes.size() != 12) {
        throw std::invalid_argument("System Event payload must be exactly 12 bytes");
    }
    if (bytes[0] != 'S') {
        throw std::invalid_argument("System Event message type must be 'S'");
    }

    return SystemEventMessage{
        detail::read_be16(bytes.subspan<1, 2>()),
        detail::read_be16(bytes.subspan<3, 2>()),
        detail::read_be48(bytes.subspan<5, 6>()),
        static_cast<char>(bytes[11]),
    };
}

}
