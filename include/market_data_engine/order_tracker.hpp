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
    void apply(const OrderExecutedMessage& message);
    void apply(const OrderExecutedWithPriceMessage& message);
    void apply(const OrderCancelMessage& message);
    void apply(const OrderDeleteMessage& message);
    void apply(const OrderReplaceMessage& message);

    std::size_t size() const noexcept;
    const ActiveOrder* find(std::uint64_t order_reference) const noexcept;

private:
    void add(ActiveOrder order);
    ActiveOrder& checked_order(std::uint64_t order_reference, std::uint16_t stock_locate);
    void reduce(std::uint64_t order_reference, std::uint16_t stock_locate,
                std::uint32_t shares);

    std::unordered_map<std::uint64_t, ActiveOrder> orders_;
};

}
