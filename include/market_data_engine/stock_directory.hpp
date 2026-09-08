#pragma once

#include <cstdint>
#include <span>
#include <string>

namespace market_data_engine {

struct StockDirectoryMessage {
    std::uint16_t stock_locate;
    std::uint16_t tracking_number;
    std::uint64_t timestamp_ns;
    std::string stock;
    char market_category;
    char financial_status_indicator;
    std::uint32_t round_lot_size;
    char round_lots_only;
    char issue_classification;
    std::string issue_sub_type;
    char authenticity;
    char short_sale_threshold_indicator;
    char ipo_flag;
    char luld_reference_price_tier;
    char etp_flag;
    std::uint32_t etp_leverage_factor; // Nasdaq rounds fractional leverage down to an integer.
    char inverse_indicator;
};

// ITCH timestamps are nanoseconds since midnight.
StockDirectoryMessage parse_stock_directory(std::span<const std::uint8_t> bytes);

}
