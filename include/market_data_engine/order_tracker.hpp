#pragma once

#include "market_data_engine/add_order.hpp"
#include "market_data_engine/modify_order.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>

namespace market_data_engine {

struct ActiveOrder {
    std::uint64_t order_reference;
    std::uint16_t stock_locate;
    char side;
    std::uint32_t remaining_shares;
    std::string stock;
    std::uint32_t price_4;
};

class OrderTracker {
public:
    void apply(const AddOrderMessage& message);
    void apply(const AddOrderWithMpidMessage& message);
    // Lifecycle messages return a copy of the pre-update order.
    ActiveOrder apply(const OrderExecutedMessage& message);
    ActiveOrder apply(const OrderExecutedWithPriceMessage& message);
    ActiveOrder apply(const OrderCancelMessage& message);
    ActiveOrder apply(const OrderDeleteMessage& message);
    ActiveOrder apply(const OrderReplaceMessage& message);

    std::size_t size() const noexcept;
    const ActiveOrder* find(std::uint64_t order_reference) const noexcept;

private:
    using Orders = std::unordered_map<std::uint64_t, ActiveOrder>;

    void add(ActiveOrder order);
    Orders::iterator checked_order(std::uint64_t order_reference, std::uint16_t stock_locate);
    ActiveOrder reduce(std::uint64_t order_reference, std::uint16_t stock_locate,
                       std::uint32_t shares);

    Orders orders_;
};

}
