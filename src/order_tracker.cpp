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
    reduce(message.order_reference, message.executed_shares);
}

void OrderTracker::apply(const OrderExecutedWithPriceMessage& message) {
    // Execution price and printability do not change the displayed order price.
    reduce(message.order_reference, message.executed_shares);
}

void OrderTracker::apply(const OrderCancelMessage& message) {
    reduce(message.order_reference, message.cancelled_shares);
}

void OrderTracker::apply(const OrderDeleteMessage& message) {
    if (orders_.erase(message.order_reference) == 0) {
        throw std::runtime_error("cannot delete an unknown order reference");
    }
}

void OrderTracker::apply(const OrderReplaceMessage& message) {
    const auto original = orders_.find(message.original_order_reference);
    if (original == orders_.end()) {
        throw std::runtime_error("cannot replace an unknown order reference");
    }
    if (orders_.contains(message.new_order_reference)) {
        throw std::runtime_error("replacement order reference is already active");
    }

    ActiveOrder replacement{
        message.new_order_reference, original->second.stock_locate,
        original->second.side, message.shares, original->second.stock,
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
    const auto reference = order.order_reference;
    if (!orders_.emplace(reference, std::move(order)).second) {
        throw std::runtime_error("cannot add a duplicate active order reference");
    }
}

void OrderTracker::reduce(std::uint64_t order_reference, std::uint32_t shares) {
    const auto order = orders_.find(order_reference);
    if (order == orders_.end()) {
        throw std::runtime_error("cannot reduce an unknown order reference");
    }
    if (shares > order->second.remaining_shares) {
        throw std::runtime_error("share reduction exceeds the remaining shares");
    }

    order->second.remaining_shares -= shares;
    if (order->second.remaining_shares == 0) {
        orders_.erase(order);
    }
}

}
