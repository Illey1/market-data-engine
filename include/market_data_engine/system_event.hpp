#pragma once

#include <cstdint>
#include <span>

namespace market_data_engine {

struct SystemEventMessage {
    std::uint16_t stock_locate;
    std::uint16_t tracking_number;
    std::uint64_t timestamp_ns;
    char event_code;
};

// Decodes a raw ITCH payload; timestamp_ns is nanoseconds since midnight.
// Throws std::invalid_argument unless bytes contains exactly 12 bytes of type 'S'.
SystemEventMessage parse_system_event(std::span<const std::uint8_t> bytes);

} // namespace market_data_engine
