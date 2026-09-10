#pragma once

#include "market_data_engine/order_tracker.hpp"
#include "market_data_engine/stock_directory.hpp"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <unordered_map>

namespace market_data_engine {

struct PriceLevel {
    std::uint32_t price_4;
    std::uint64_t shares;
};

class OrderBook {
public:
    void apply(const StockDirectoryMessage& message);
    void apply(const AddOrderMessage& message);
    void apply(const AddOrderWithMpidMessage& message);
    void apply(const OrderExecutedMessage& message);
    void apply(const OrderExecutedWithPriceMessage& message);
    void apply(const OrderCancelMessage& message);
    void apply(const OrderDeleteMessage& message);
    void apply(const OrderReplaceMessage& message);

    std::optional<PriceLevel> best_bid(const std::string& stock) const;
    std::optional<PriceLevel> best_ask(const std::string& stock) const;
    std::uint64_t quantity_at(const std::string& stock, char side,
                              std::uint32_t price_4) const;
    std::size_t active_order_count() const noexcept;

private:
    struct SymbolBook {
        std::map<std::uint32_t, std::uint64_t, std::greater<>> bids;
        std::map<std::uint32_t, std::uint64_t> asks;
    };

    void validate_stock(std::uint16_t stock_locate, const std::string& stock) const;
    void add_level(const std::string& stock, char side, std::uint32_t price_4,
                   std::uint64_t shares);
    void remove_level(const std::string& stock, char side, std::uint32_t price_4,
                      std::uint64_t shares);

    OrderTracker tracker_;
    std::unordered_map<std::string, SymbolBook> books_;
    std::unordered_map<std::uint16_t, std::string> stock_directory_;
};

}
