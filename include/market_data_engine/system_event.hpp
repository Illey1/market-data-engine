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

// ITCH timestamps are nanoseconds since midnight.
SystemEventMessage parse_system_event(std::span<const std::uint8_t> bytes);

}
