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

ActiveOrder OrderTracker::apply(const OrderExecutedMessage& message) {
    return reduce(message.order_reference, message.stock_locate, message.executed_shares);
}

ActiveOrder OrderTracker::apply(const OrderExecutedWithPriceMessage& message) {
    // Execution price and printability do not change the displayed order price.
    return reduce(message.order_reference, message.stock_locate, message.executed_shares);
}

ActiveOrder OrderTracker::apply(const OrderCancelMessage& message) {
    return reduce(message.order_reference, message.stock_locate, message.cancelled_shares);
}

ActiveOrder OrderTracker::apply(const OrderDeleteMessage& message) {
    const auto order = checked_order(message.order_reference, message.stock_locate);
    ActiveOrder original = order->second;
    orders_.erase(order);
    return original;
}

ActiveOrder OrderTracker::apply(const OrderReplaceMessage& message) {
    const auto order = checked_order(message.original_order_reference,
                                     message.stock_locate);
    if (orders_.contains(message.new_order_reference)) {
        throw std::runtime_error("replacement order reference is already active");
    }

    ActiveOrder original = order->second;
    ActiveOrder replacement{
        message.new_order_reference, original.stock_locate,
        original.side, message.shares, original.stock,
        message.price_4,
    };
    orders_.emplace(message.new_order_reference, std::move(replacement));
    // Insertion can rehash orders_, so erase the original by key.
    orders_.erase(message.original_order_reference);
    return original;
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

OrderTracker::Orders::iterator OrderTracker::checked_order(
    std::uint64_t order_reference, std::uint16_t stock_locate) {
    const auto order = orders_.find(order_reference);
    if (order == orders_.end()) {
        throw std::runtime_error("unknown order reference");
    }
    if (order->second.stock_locate != stock_locate) {
        throw std::runtime_error("stock locate does not match the active order");
    }
    return order;
}

ActiveOrder OrderTracker::reduce(std::uint64_t order_reference, std::uint16_t stock_locate,
                                 std::uint32_t shares) {
    const auto order = checked_order(order_reference, stock_locate);
    if (shares > order->second.remaining_shares) {
        throw std::runtime_error("share reduction exceeds the remaining shares");
    }

    ActiveOrder original = order->second;
    order->second.remaining_shares -= shares;
    if (order->second.remaining_shares == 0) {
        orders_.erase(order);
    }
    return original;
}

}
