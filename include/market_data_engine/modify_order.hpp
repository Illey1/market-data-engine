#pragma once

#include <cstdint>
#include <span>

namespace market_data_engine {

struct OrderExecutedMessage {
    std::uint16_t stock_locate;
    std::uint16_t tracking_number;
    std::uint64_t timestamp_ns;
    std::uint64_t order_reference;
    std::uint32_t executed_shares;
    std::uint64_t match_number;
};

struct OrderExecutedWithPriceMessage {
    std::uint16_t stock_locate;
    std::uint16_t tracking_number;
    std::uint64_t timestamp_ns;
    std::uint64_t order_reference;
    std::uint32_t executed_shares;
    std::uint64_t match_number;
    char printable;
    std::uint32_t execution_price_4; // Price(4) units are 0.0001.
};

struct OrderCancelMessage {
    std::uint16_t stock_locate;
    std::uint16_t tracking_number;
    std::uint64_t timestamp_ns;
    std::uint64_t order_reference;
    std::uint32_t cancelled_shares;
};

struct OrderDeleteMessage {
    std::uint16_t stock_locate;
    std::uint16_t tracking_number;
    std::uint64_t timestamp_ns;
    std::uint64_t order_reference;
};

struct OrderReplaceMessage {
    std::uint16_t stock_locate;
    std::uint16_t tracking_number;
    std::uint64_t timestamp_ns;
    std::uint64_t original_order_reference;
    std::uint64_t new_order_reference;
    std::uint32_t shares;
    std::uint32_t price_4; // Price(4) units are 0.0001.
};

OrderExecutedMessage parse_order_executed(std::span<const std::uint8_t> bytes);
OrderExecutedWithPriceMessage parse_order_executed_with_price(
    std::span<const std::uint8_t> bytes);
OrderCancelMessage parse_order_cancel(std::span<const std::uint8_t> bytes);
OrderDeleteMessage parse_order_delete(std::span<const std::uint8_t> bytes);
OrderReplaceMessage parse_order_replace(std::span<const std::uint8_t> bytes);

}
