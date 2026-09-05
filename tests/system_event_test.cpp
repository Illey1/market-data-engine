#include "market_data_engine/system_event.hpp"

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
    const auto check_invalid = [&check](std::span<const std::uint8_t> bytes,
                                        const char* description) {
        try {
            (void)market_data_engine::parse_system_event(bytes);
            check(false, description);
        } catch (const std::invalid_argument&) {
            // The parser must reject malformed payloads with this exception.
        } catch (...) {
            check(false, "malformed payload threw the wrong exception type");
        }
    };

    const std::array<std::uint8_t, 12> payload{
        'S', 0x12, 0x34, 0xab, 0xcd, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 'O'};
    const auto message = market_data_engine::parse_system_event(payload);
    check(message.stock_locate == 0x1234, "stock locate is big-endian");
    check(message.tracking_number == 0xabcd, "tracking number is big-endian");
    check(message.timestamp_ns == 0x000102030405ULL,
          "all six timestamp bytes are read in big-endian order");
    check(message.event_code == 'O', "event code is read from the last byte");

    const std::array<std::uint8_t, 12> maximum_payload{
        'S', 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, '?'};
    const auto maximum = market_data_engine::parse_system_event(maximum_payload);
    check(maximum.stock_locate == 0xffff, "maximum unsigned stock locate");
    check(maximum.tracking_number == 0xffff, "maximum unsigned tracking number");
    check(maximum.timestamp_ns == 0xffffffffffffULL,
          "maximum 48-bit timestamp includes its uppermost byte");
    check(maximum.event_code == '?', "unknown ASCII event codes are accepted");

    auto high_timestamp_payload = payload;
    high_timestamp_payload[5] = 0x80;
    const auto high_timestamp =
        market_data_engine::parse_system_event(high_timestamp_payload);
    check(high_timestamp.timestamp_ns == 0x800102030405ULL,
          "uppermost timestamp bit is unsigned and preserved");

    check_invalid({}, "empty payload must be rejected");
    for (std::size_t length = 1; length < payload.size(); ++length) {
        check_invalid(std::span<const std::uint8_t>{payload}.first(length),
                      "short payload must be rejected");
    }

    const std::array<std::uint8_t, 13> oversized_payload{
        'S', 0x12, 0x34, 0xab, 0xcd, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 'O', 0x00};
    check_invalid(oversized_payload, "oversized payload must be rejected");

    auto wrong_type_payload = payload;
    wrong_type_payload[0] = 'A';
    check_invalid(wrong_type_payload, "wrong message type must be rejected");
    wrong_type_payload[0] = 's';
    check_invalid(wrong_type_payload, "message type is case-sensitive");

    if (failures != 0) {
        std::cerr << failures << " check(s) failed\n";
        return 1;
    }
    std::cout << "All System Event parser checks passed\n";
    return 0;
}
