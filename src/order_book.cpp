#include "market_data_engine/order_book.hpp"

#include <stdexcept>

namespace market_data_engine {

void OrderBook::apply(const StockDirectoryMessage& message) {
    if (!stock_directory_.emplace(message.stock_locate, message.stock).second) {
        throw std::runtime_error("duplicate Stock Directory stock locate");
    }
}

void OrderBook::apply(const AddOrderMessage& message) {
    validate_stock(message.stock_locate, message.stock);
    tracker_.apply(message);
    add_level(message.stock, message.side, message.price_4, message.shares);
}

void OrderBook::apply(const AddOrderWithMpidMessage& message) {
    validate_stock(message.stock_locate, message.stock);
    tracker_.apply(message);
    add_level(message.stock, message.side, message.price_4, message.shares);
}

void OrderBook::apply(const OrderExecutedMessage& message) {
    const auto original = original_order(message.order_reference);
    tracker_.apply(message);
    remove_level(original.stock, original.side, original.price_4,
                 message.executed_shares);
}

void OrderBook::apply(const OrderExecutedWithPriceMessage& message) {
    const auto original = original_order(message.order_reference);
    tracker_.apply(message);
    // C executions reduce the original displayed level, regardless of execution price.
    remove_level(original.stock, original.side, original.price_4,
                 message.executed_shares);
}

void OrderBook::apply(const OrderCancelMessage& message) {
    const auto original = original_order(message.order_reference);
    tracker_.apply(message);
    remove_level(original.stock, original.side, original.price_4,
                 message.cancelled_shares);
}

void OrderBook::apply(const OrderDeleteMessage& message) {
    const auto original = original_order(message.order_reference);
    tracker_.apply(message);
    remove_level(original.stock, original.side, original.price_4,
                 original.remaining_shares);
}

void OrderBook::apply(const OrderReplaceMessage& message) {
    const auto original = original_order(message.original_order_reference);
    tracker_.apply(message);
    remove_level(original.stock, original.side, original.price_4,
                 original.remaining_shares);
    add_level(original.stock, original.side, message.price_4, message.shares);
}

std::optional<PriceLevel> OrderBook::best_bid(const std::string& stock) const {
    const auto symbol = books_.find(stock);
    if (symbol == books_.end() || symbol->second.bids.empty()) {
        return std::nullopt;
    }
    const auto& [price, shares] = *symbol->second.bids.begin();
    return PriceLevel{price, shares};
}

std::optional<PriceLevel> OrderBook::best_ask(const std::string& stock) const {
    const auto symbol = books_.find(stock);
    if (symbol == books_.end() || symbol->second.asks.empty()) {
        return std::nullopt;
    }
    const auto& [price, shares] = *symbol->second.asks.begin();
    return PriceLevel{price, shares};
}

std::uint64_t OrderBook::quantity_at(const std::string& stock, char side,
                                    std::uint32_t price_4) const {
    const auto symbol = books_.find(stock);
    if (symbol == books_.end()) {
        return 0;
    }
    const auto quantity = [price_4](const auto& levels) -> std::uint64_t {
        const auto level = levels.find(price_4);
        return level == levels.end() ? 0 : level->second;
    };
    return side == 'B' ? quantity(symbol->second.bids) : quantity(symbol->second.asks);
}

std::size_t OrderBook::active_order_count() const noexcept {
    return tracker_.size();
}

void OrderBook::validate_stock(std::uint16_t stock_locate, const std::string& stock) const {
    const auto entry = stock_directory_.find(stock_locate);
    if (entry != stock_directory_.end() && entry->second != stock) {
        throw std::runtime_error("Add stock does not match the Stock Directory symbol");
    }
}

ActiveOrder OrderBook::original_order(std::uint64_t order_reference) const {
    const auto* order = tracker_.find(order_reference);
    if (order == nullptr) {
        throw std::runtime_error("unknown order reference");
    }
    // Preserve the fields even if the tracker erases or replaces this order.
    return *order;
}

void OrderBook::add_level(const std::string& stock, char side,
                          std::uint32_t price_4, std::uint64_t shares) {
    if (shares == 0) {
        return;
    }
    auto& symbol = books_[stock];
    if (side == 'B') {
        symbol.bids[price_4] += shares;
    } else {
        symbol.asks[price_4] += shares;
    }
}

void OrderBook::remove_level(const std::string& stock, char side,
                             std::uint32_t price_4, std::uint64_t shares) {
    if (shares == 0) {
        return;
    }
    const auto symbol = books_.find(stock);
    if (symbol == books_.end()) {
        throw std::logic_error("cannot reduce a missing symbol's price level");
    }

    const auto remove = [price_4, shares](auto& levels) {
        const auto level = levels.find(price_4);
        if (level == levels.end() || level->second < shares) {
            throw std::logic_error("price level quantity is smaller than the reduction");
        }
        level->second -= shares;
        if (level->second == 0) {
            levels.erase(level);
        }
    };
    if (side == 'B') {
        remove(symbol->second.bids);
    } else {
        remove(symbol->second.asks);
    }
    if (symbol->second.bids.empty() && symbol->second.asks.empty()) {
        books_.erase(symbol);
    }
}

}
