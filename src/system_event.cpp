#include "market_data_engine/system_event.hpp"

#include <stdexcept>

namespace market_data_engine {
namespace {

std::uint16_t read_be16(std::span<const std::uint8_t, 2> bytes) {
    return static_cast<std::uint16_t>(
        (static_cast<std::uint16_t>(bytes[0]) << 8) | bytes[1]);
}

std::uint64_t read_be48(std::span<const std::uint8_t, 6> bytes) {
    std::uint64_t value = 0;
    for (const std::uint8_t byte : bytes) {
        value = (value << 8) | static_cast<std::uint64_t>(byte);
    }
    return value;
}

}

SystemEventMessage parse_system_event(std::span<const std::uint8_t> bytes) {
    if (bytes.size() != 12) {
        throw std::invalid_argument("System Event payload must be exactly 12 bytes");
    }
    if (bytes[0] != 'S') {
        throw std::invalid_argument("System Event message type must be 'S'");
    }

    return SystemEventMessage{
        read_be16(bytes.subspan<1, 2>()),
        read_be16(bytes.subspan<3, 2>()),
        read_be48(bytes.subspan<5, 6>()),
        static_cast<char>(bytes[11]),
    };
}

}
