#pragma once

#include <cstdint>
#include <span>

namespace market_data_engine::detail {

inline std::uint16_t read_be16(std::span<const std::uint8_t, 2> bytes) {
    return static_cast<std::uint16_t>(
        (static_cast<std::uint16_t>(bytes[0]) << 8) | bytes[1]);
}

inline std::uint32_t read_be32(std::span<const std::uint8_t, 4> bytes) {
    std::uint32_t value = 0;
    for (const std::uint8_t byte : bytes) {
        value = (value << 8) | static_cast<std::uint32_t>(byte);
    }
    return value;
}

inline std::uint64_t read_be48(std::span<const std::uint8_t, 6> bytes) {
    std::uint64_t value = 0;
    for (const std::uint8_t byte : bytes) {
        value = (value << 8) | static_cast<std::uint64_t>(byte);
    }
    return value;
}

inline std::uint64_t read_be64(std::span<const std::uint8_t, 8> bytes) {
    std::uint64_t value = 0;
    for (const std::uint8_t byte : bytes) {
        value = (value << 8) | static_cast<std::uint64_t>(byte);
    }
    return value;
}

}
