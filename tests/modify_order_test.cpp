#include "market_data_engine/modify_order.hpp"

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
    const auto check_invalid = [&check](auto parser,
                                        std::span<const std::uint8_t> bytes,
                                        const char* description) {
        try {
            (void)parser(bytes);
            check(false, description);
        } catch (const std::invalid_argument&) {
        } catch (...) {
            check(false, "malformed payload threw the wrong exception type");
        }
    };

    std::array<std::uint8_t, 31> executed_payload{
        'E',
        0x12, 0x34,
        0xab, 0xcd,
        0x81, 0x02, 0x03, 0x04, 0x05, 0x06,
        0x82, 0x13, 0x24, 0x35, 0x46, 0x57, 0x68, 0x79,
        0x83, 0x14, 0x25, 0x36,
        0x84, 0x15, 0x26, 0x37, 0x48, 0x59, 0x6a, 0x7b,
    };
    const auto executed = market_data_engine::parse_order_executed(executed_payload);
    check(executed.stock_locate == 0x1234, "E stock locate");
    check(executed.tracking_number == 0xabcd, "E tracking number");
    check(executed.timestamp_ns == 0x810203040506ULL, "E six-byte timestamp");
    check(executed.order_reference == 0x8213243546576879ULL, "E unsigned order reference");
    check(executed.executed_shares == 0x83142536U, "E unsigned executed shares");
    check(executed.match_number == 0x8415263748596a7bULL, "E unsigned match number");
    check_invalid(market_data_engine::parse_order_executed,
                  std::span<const std::uint8_t>{executed_payload}.first(30),
                  "E incorrect length is rejected");
    executed_payload[0] = 'A';
    check_invalid(market_data_engine::parse_order_executed, executed_payload,
                  "E incorrect message type is rejected");

    std::array<std::uint8_t, 36> priced_execution_payload{
        'C',
        0x23, 0x45,
        0xbc, 0xde,
        0x82, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x83, 0x14, 0x25, 0x36, 0x47, 0x58, 0x69, 0x7a,
        0x84, 0x15, 0x26, 0x37,
        0x85, 0x16, 0x27, 0x38, 0x49, 0x5a, 0x6b, 0x7c,
        'Y',
        0x86, 0x17, 0x28, 0x39,
    };
    const auto priced_execution =
        market_data_engine::parse_order_executed_with_price(priced_execution_payload);
    check(priced_execution.stock_locate == 0x2345, "C stock locate");
    check(priced_execution.tracking_number == 0xbcde, "C tracking number");
    check(priced_execution.timestamp_ns == 0x820304050607ULL, "C six-byte timestamp");
    check(priced_execution.order_reference == 0x831425364758697aULL,
          "C unsigned order reference");
    check(priced_execution.executed_shares == 0x84152637U, "C unsigned executed shares");
    check(priced_execution.match_number == 0x85162738495a6b7cULL,
          "C unsigned match number");
    check(priced_execution.printable == 'Y', "C printable indicator");
    check(priced_execution.execution_price_4 == 0x86172839U,
          "C exact unsigned Price(4) integer");
    check_invalid(market_data_engine::parse_order_executed_with_price,
                  std::span<const std::uint8_t>{priced_execution_payload}.first(35),
                  "C incorrect length is rejected");
    priced_execution_payload[0] = 'A';
    check_invalid(market_data_engine::parse_order_executed_with_price,
                  priced_execution_payload, "C incorrect message type is rejected");

    std::array<std::uint8_t, 23> cancel_payload{
        'X',
        0x34, 0x56,
        0xcd, 0xef,
        0x83, 0x04, 0x05, 0x06, 0x07, 0x08,
        0x84, 0x15, 0x26, 0x37, 0x48, 0x59, 0x6a, 0x7b,
        0x85, 0x16, 0x27, 0x38,
    };
    const auto cancel = market_data_engine::parse_order_cancel(cancel_payload);
    check(cancel.stock_locate == 0x3456, "X stock locate");
    check(cancel.tracking_number == 0xcdef, "X tracking number");
    check(cancel.timestamp_ns == 0x830405060708ULL, "X six-byte timestamp");
    check(cancel.order_reference == 0x8415263748596a7bULL, "X unsigned order reference");
    check(cancel.cancelled_shares == 0x85162738U, "X unsigned cancelled shares");
    check_invalid(market_data_engine::parse_order_cancel,
                  std::span<const std::uint8_t>{cancel_payload}.first(22),
                  "X incorrect length is rejected");
    cancel_payload[0] = 'A';
    check_invalid(market_data_engine::parse_order_cancel, cancel_payload,
                  "X incorrect message type is rejected");

    std::array<std::uint8_t, 19> delete_payload{
        'D',
        0x45, 0x67,
        0xde, 0xf0,
        0x84, 0x05, 0x06, 0x07, 0x08, 0x09,
        0x85, 0x16, 0x27, 0x38, 0x49, 0x5a, 0x6b, 0x7c,
    };
    const auto deleted = market_data_engine::parse_order_delete(delete_payload);
    check(deleted.stock_locate == 0x4567, "D stock locate");
    check(deleted.tracking_number == 0xdef0, "D tracking number");
    check(deleted.timestamp_ns == 0x840506070809ULL, "D six-byte timestamp");
    check(deleted.order_reference == 0x85162738495a6b7cULL, "D unsigned order reference");
    check_invalid(market_data_engine::parse_order_delete,
                  std::span<const std::uint8_t>{delete_payload}.first(18),
                  "D incorrect length is rejected");
    delete_payload[0] = 'A';
    check_invalid(market_data_engine::parse_order_delete, delete_payload,
                  "D incorrect message type is rejected");

    std::array<std::uint8_t, 35> replace_payload{
        'U',
        0x56, 0x78,
        0xef, 0x01,
        0x85, 0x06, 0x07, 0x08, 0x09, 0x0a,
        0x86, 0x17, 0x28, 0x39, 0x4a, 0x5b, 0x6c, 0x7d,
        0x97, 0x28, 0x39, 0x4a, 0x5b, 0x6c, 0x7d, 0x8e,
        0xa8, 0x39, 0x4a, 0x5b,
        0xb9, 0x4a, 0x5b, 0x6c,
    };
    const auto replaced = market_data_engine::parse_order_replace(replace_payload);
    check(replaced.stock_locate == 0x5678, "U stock locate");
    check(replaced.tracking_number == 0xef01, "U tracking number");
    check(replaced.timestamp_ns == 0x85060708090aULL, "U six-byte timestamp");
    check(replaced.original_order_reference == 0x861728394a5b6c7dULL,
          "U unsigned original order reference");
    check(replaced.new_order_reference == 0x9728394a5b6c7d8eULL,
          "U unsigned new order reference");
    check(replaced.shares == 0xa8394a5bU, "U unsigned replacement shares");
    check(replaced.price_4 == 0xb94a5b6cU, "U exact unsigned Price(4) integer");
    check_invalid(market_data_engine::parse_order_replace,
                  std::span<const std::uint8_t>{replace_payload}.first(34),
                  "U incorrect length is rejected");
    replace_payload[0] = 'A';
    check_invalid(market_data_engine::parse_order_replace, replace_payload,
                  "U incorrect message type is rejected");

    if (failures != 0) {
        std::cerr << failures << " check(s) failed\n";
        return 1;
    }
    std::cout << "All modify-order parser checks passed\n";
    return 0;
}
