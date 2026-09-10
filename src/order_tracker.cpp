#include "market_data_engine/order_tracker.hpp"

#include <stdexcept>
#include <utility>

namespace market_data_engine {

void OrderTracker::apply(const AddOrderMessage& message) {
    add({message.order_reference, message.stock_locate, message.side,
         message.shares, message.stock, message.price_4});
}

void OrderTracker::apply(const AddOrderWithMpidMessage& message) {
    add({message.order_reference, message.stock_locate, message.side,
         message.shares, message.stock, message.price_4});
}

void OrderTracker::apply(const OrderExecutedMessage& message) {
    reduce(message.order_reference, message.stock_locate, message.executed_shares);
}

void OrderTracker::apply(const OrderExecutedWithPriceMessage& message) {
    // Execution price and printability do not change the displayed order price.
    reduce(message.order_reference, message.stock_locate, message.executed_shares);
}

void OrderTracker::apply(const OrderCancelMessage& message) {
    reduce(message.order_reference, message.stock_locate, message.cancelled_shares);
}

void OrderTracker::apply(const OrderDeleteMessage& message) {
    checked_order(message.order_reference, message.stock_locate);
    orders_.erase(message.order_reference);
}

void OrderTracker::apply(const OrderReplaceMessage& message) {
    const auto& original = checked_order(message.original_order_reference,
                                         message.stock_locate);
    if (orders_.contains(message.new_order_reference)) {
        throw std::runtime_error("replacement order reference is already active");
    }

    ActiveOrder replacement{
        message.new_order_reference, original.stock_locate,
        original.side, message.shares, original.stock,
        message.price_4,
    };
    orders_.emplace(message.new_order_reference, std::move(replacement));
    orders_.erase(message.original_order_reference);
}

std::size_t OrderTracker::size() const noexcept {
    return orders_.size();
}

const ActiveOrder* OrderTracker::find(std::uint64_t order_reference) const noexcept {
    const auto order = orders_.find(order_reference);
    return order == orders_.end() ? nullptr : &order->second;
}

void OrderTracker::add(ActiveOrder order) {
    if (order.side != 'B' && order.side != 'S') {
        throw std::runtime_error("Add buy/sell indicator must be 'B' or 'S'");
    }
    const auto reference = order.order_reference;
    if (!orders_.emplace(reference, std::move(order)).second) {
        throw std::runtime_error("cannot add a duplicate active order reference");
    }
}

ActiveOrder& OrderTracker::checked_order(std::uint64_t order_reference,
                                         std::uint16_t stock_locate) {
    const auto order = orders_.find(order_reference);
    if (order == orders_.end()) {
        throw std::runtime_error("unknown order reference");
    }
    if (order->second.stock_locate != stock_locate) {
        throw std::runtime_error("stock locate does not match the active order");
    }
    return order->second;
}

void OrderTracker::reduce(std::uint64_t order_reference, std::uint16_t stock_locate,
                           std::uint32_t shares) {
    auto& order = checked_order(order_reference, stock_locate);
    if (shares > order.remaining_shares) {
        throw std::runtime_error("share reduction exceeds the remaining shares");
    }

    order.remaining_shares -= shares;
    if (order.remaining_shares == 0) {
        orders_.erase(order_reference);
    }
}

}
