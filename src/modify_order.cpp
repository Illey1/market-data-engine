#include "market_data_engine/modify_order.hpp"

#include "big_endian.hpp"

#include <stdexcept>

namespace market_data_engine {

OrderExecutedMessage parse_order_executed(std::span<const std::uint8_t> bytes) {
    if (bytes.size() != 31) {
        throw std::invalid_argument("Order Executed payload must be exactly 31 bytes");
    }
    if (bytes[0] != 'E') {
        throw std::invalid_argument("Order Executed message type must be 'E'");
    }

    return OrderExecutedMessage{
        detail::read_be16(bytes.subspan<1, 2>()),
        detail::read_be16(bytes.subspan<3, 2>()),
        detail::read_be48(bytes.subspan<5, 6>()),
        detail::read_be64(bytes.subspan<11, 8>()),
        detail::read_be32(bytes.subspan<19, 4>()),
        detail::read_be64(bytes.subspan<23, 8>()),
    };
}

OrderExecutedWithPriceMessage parse_order_executed_with_price(
    std::span<const std::uint8_t> bytes) {
    if (bytes.size() != 36) {
        throw std::invalid_argument(
            "Order Executed With Price payload must be exactly 36 bytes");
    }
    if (bytes[0] != 'C') {
        throw std::invalid_argument("Order Executed With Price message type must be 'C'");
    }

    return OrderExecutedWithPriceMessage{
        detail::read_be16(bytes.subspan<1, 2>()),
        detail::read_be16(bytes.subspan<3, 2>()),
        detail::read_be48(bytes.subspan<5, 6>()),
        detail::read_be64(bytes.subspan<11, 8>()),
        detail::read_be32(bytes.subspan<19, 4>()),
        detail::read_be64(bytes.subspan<23, 8>()),
        static_cast<char>(bytes[31]),
        detail::read_be32(bytes.subspan<32, 4>()),
    };
}

OrderCancelMessage parse_order_cancel(std::span<const std::uint8_t> bytes) {
    if (bytes.size() != 23) {
        throw std::invalid_argument("Order Cancel payload must be exactly 23 bytes");
    }
    if (bytes[0] != 'X') {
        throw std::invalid_argument("Order Cancel message type must be 'X'");
    }

    return OrderCancelMessage{
        detail::read_be16(bytes.subspan<1, 2>()),
        detail::read_be16(bytes.subspan<3, 2>()),
        detail::read_be48(bytes.subspan<5, 6>()),
        detail::read_be64(bytes.subspan<11, 8>()),
        detail::read_be32(bytes.subspan<19, 4>()),
    };
}

OrderDeleteMessage parse_order_delete(std::span<const std::uint8_t> bytes) {
    if (bytes.size() != 19) {
        throw std::invalid_argument("Order Delete payload must be exactly 19 bytes");
    }
    if (bytes[0] != 'D') {
        throw std::invalid_argument("Order Delete message type must be 'D'");
    }

    return OrderDeleteMessage{
        detail::read_be16(bytes.subspan<1, 2>()),
        detail::read_be16(bytes.subspan<3, 2>()),
        detail::read_be48(bytes.subspan<5, 6>()),
        detail::read_be64(bytes.subspan<11, 8>()),
    };
}

OrderReplaceMessage parse_order_replace(std::span<const std::uint8_t> bytes) {
    if (bytes.size() != 35) {
        throw std::invalid_argument("Order Replace payload must be exactly 35 bytes");
    }
    if (bytes[0] != 'U') {
        throw std::invalid_argument("Order Replace message type must be 'U'");
    }

    return OrderReplaceMessage{
        detail::read_be16(bytes.subspan<1, 2>()),
        detail::read_be16(bytes.subspan<3, 2>()),
        detail::read_be48(bytes.subspan<5, 6>()),
        detail::read_be64(bytes.subspan<11, 8>()),
        detail::read_be64(bytes.subspan<19, 8>()),
        detail::read_be32(bytes.subspan<27, 4>()),
        detail::read_be32(bytes.subspan<31, 4>()),
    };
}

}
