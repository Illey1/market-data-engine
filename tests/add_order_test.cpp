#include "market_data_engine/add_order.hpp"

#include <array>
#include <cstdint>
#include <iostream>
#include <span>
#include <stdexcept>

int main() {
    int failures = 0;
    const auto check = [&failures](bool condition, const char* description) {
        if (!condition) {
            std::cerr << "FAIL: " << description << '\n';
            ++failures;
        }
    };
    const auto check_invalid = [&check](auto parse,
                                        std::span<const std::uint8_t> bytes,
                                        const char* description) {
        try {
            (void)parse(bytes);
            check(false, description);
        } catch (const std::invalid_argument&) {
        } catch (...) {
            check(false, "malformed payload threw the wrong exception type");
        }
    };

    const std::array<std::uint8_t, 36> payload{
        'A',
        0x12, 0x34,
        0xab, 0xcd,
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
        0x81, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
        'B',
        0x00, 0x01, 0x02, 0x03,
        'A', 'A', 'P', 'L', ' ', ' ', ' ', ' ',
        0x00, 0x12, 0xd6, 0x87,
    };
    const auto message = market_data_engine::parse_add_order(payload);
    check(message.stock_locate == 0x1234, "stock locate is big-endian");
    check(message.tracking_number == 0xabcd, "tracking number is big-endian");
    check(message.timestamp_ns == 0x000102030405ULL,
          "all six timestamp bytes are read in big-endian order");
    check(message.order_reference == 0x8123456789abcdefULL,
          "all eight order reference bytes preserve unsigned big-endian value");
    check(message.side == 'B', "buy/sell indicator is decoded");
    check(message.shares == 66051, "shares are big-endian");
    check(message.stock == "AAPL", "stock trailing padding spaces are removed");
    check(message.price_4 == 1234567, "price retains the exact Price(4) integer");

    check_invalid(market_data_engine::parse_add_order,
                  std::span<const std::uint8_t>{payload}.first(35),
                  "short payload must be rejected");

    auto wrong_type_payload = payload;
    wrong_type_payload[0] = 'S';
    check_invalid(market_data_engine::parse_add_order, wrong_type_payload,
                  "wrong message type must be rejected");

    const std::array<std::uint8_t, 40> mpid_payload{
        'F',
        0x23, 0x45,
        0xbc, 0xde,
        0x80, 0x01, 0x02, 0x03, 0x04, 0x05,
        0x92, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0,
        'S',
        0x01, 0x02, 0x03, 0x04,
        'M', 'S', 'F', 'T', ' ', ' ', ' ', ' ',
        0x00, 0x25, 0xad, 0x0e,
        'G', 'S', ' ', ' ',
    };
    const auto mpid_message = market_data_engine::parse_add_order_with_mpid(mpid_payload);
    check(mpid_message.stock_locate == 0x2345, "F stock locate is big-endian");
    check(mpid_message.tracking_number == 0xbcde, "F tracking number is big-endian");
    check(mpid_message.timestamp_ns == 0x800102030405ULL,
          "F timestamp preserves the unsigned six-byte value");
    check(mpid_message.order_reference == 0x923456789abcdef0ULL,
          "F order reference preserves the unsigned eight-byte value");
    check(mpid_message.side == 'S', "F buy/sell indicator is decoded");
    check(mpid_message.shares == 0x01020304, "F shares are big-endian");
    check(mpid_message.stock == "MSFT", "F stock trailing padding spaces are removed");
    check(mpid_message.price_4 == 2469134, "F price retains the exact Price(4) integer");
    check(mpid_message.attribution == "GS", "F attribution trailing padding spaces are removed");

    check_invalid(market_data_engine::parse_add_order_with_mpid,
                  std::span<const std::uint8_t>{mpid_payload}.first(39),
                  "short F payload must be rejected");
    auto wrong_mpid_type_payload = mpid_payload;
    wrong_mpid_type_payload[0] = 'A';
    check_invalid(market_data_engine::parse_add_order_with_mpid, wrong_mpid_type_payload,
                  "wrong F message type must be rejected");

    if (failures != 0) {
        std::cerr << failures << " check(s) failed\n";
        return 1;
    }
    std::cout << "All Add Order parser checks passed\n";
    return 0;
}
