#include "market_data_engine/stock_directory.hpp"

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
            (void)market_data_engine::parse_stock_directory(bytes);
            check(false, description);
        } catch (const std::invalid_argument&) {
        } catch (...) {
            check(false, "malformed payload threw the wrong exception type");
        }
    };

    const std::array<std::uint8_t, 39> payload{
        'R',
        0x12, 0x34,
        0xab, 0xcd,
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
        'T', 'E', 'S', 'T', ' ', ' ', ' ', ' ',
        'Q',
        'N',
        0x00, 0x01, 0x02, 0x03,
        'N',
        'Q',
        'I', ' ',
        'T',
        'N',
        'Z',
        '1',
        'Y',
        0x00, 0x00, 0x00, 0x03,
        'Y',
    };
    const auto message = market_data_engine::parse_stock_directory(payload);
    check(message.stock_locate == 0x1234, "stock locate is big-endian");
    check(message.tracking_number == 0xabcd, "tracking number is big-endian");
    check(message.timestamp_ns == 0x010203040506ULL,
          "all six timestamp bytes are read in big-endian order");
    check(message.stock == "TEST", "stock trailing padding spaces are removed");
    check(message.market_category == 'Q', "market category is decoded");
    check(message.financial_status_indicator == 'N', "financial status is decoded");
    check(message.round_lot_size == 66051, "round lot size is big-endian");
    check(message.round_lots_only == 'N', "round lots only indicator is decoded");
    check(message.issue_classification == 'Q', "issue classification is decoded");
    check(message.issue_sub_type == "I", "issue sub-type trailing padding is removed");
    check(message.authenticity == 'T', "authenticity is decoded");
    check(message.short_sale_threshold_indicator == 'N', "short sale threshold is decoded");
    check(message.ipo_flag == 'Z', "non-IPO new listing flag is preserved");
    check(message.luld_reference_price_tier == '1', "LULD reference price tier is decoded");
    check(message.etp_flag == 'Y', "ETP flag is decoded");
    check(message.etp_leverage_factor == 3, "ETP leverage retains its integer value");
    check(message.inverse_indicator == 'Y', "inverse indicator is decoded");

    check_invalid(std::span<const std::uint8_t>{payload}.first(38),
                  "short payload must be rejected");
    auto wrong_type_payload = payload;
    wrong_type_payload[0] = 'A';
    check_invalid(wrong_type_payload, "wrong message type must be rejected");

    if (failures != 0) {
        std::cerr << failures << " check(s) failed\n";
        return 1;
    }
    std::cout << "All Stock Directory parser checks passed\n";
    return 0;
}
