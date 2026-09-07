#pragma once

#include <cstdint>
#include <span>
#include <string>

namespace market_data_engine {

struct AddOrderMessage {
    std::uint16_t stock_locate;
    std::uint16_t tracking_number;
    std::uint64_t timestamp_ns;
    std::uint64_t order_reference;
    char side;
    std::uint32_t shares;
    std::string stock;
    std::uint32_t price_4; // Price(4) units are 0.0001.
};

// ITCH timestamps are nanoseconds since midnight.
AddOrderMessage parse_add_order(std::span<const std::uint8_t> bytes);

}
