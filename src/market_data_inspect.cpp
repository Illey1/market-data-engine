#include "market_data_engine/add_order.hpp"
#include "market_data_engine/binary_file.hpp"
#include "market_data_engine/modify_order.hpp"
#include "market_data_engine/order_book.hpp"
#include "market_data_engine/stock_directory.hpp"
#include "market_data_engine/system_event.hpp"

#include <charconv>
#include <cstdint>
#include <exception>
#include <fstream>
#include <iostream>
#include <string_view>
#include <system_error>
#include <vector>

int main(int argc, char* argv[]) {
    if (argc != 2 && argc != 4) {
        std::cerr << "Usage: " << argv[0]
                  << " <decompressed-binaryfile|-> [--max-messages N]\n";
        return 1;
    }

    std::uint64_t max_messages = 0;
    if (argc == 4) {
        if (std::string_view(argv[2]) != "--max-messages") {
            std::cerr << "Unknown option: " << argv[2] << '\n';
            return 1;
        }
        const std::string_view value(argv[3]);
        const auto result = std::from_chars(
            value.data(), value.data() + value.size(), max_messages);
        if (result.ec != std::errc{} || result.ptr != value.data() + value.size()
            || max_messages == 0) {
            std::cerr << "Invalid --max-messages value: " << value << '\n';
            return 1;
        }
    }

    try {
        std::ifstream file;
        std::istream* input = &std::cin;
        if (std::string_view(argv[1]) != "-") {
            file.open(argv[1], std::ios::binary);
            if (!file) {
                std::cerr << "Cannot open file: " << argv[1] << '\n';
                return 1;
            }
            input = &file;
        }

        std::uint64_t messages = 0;
        std::uint64_t system_events = 0;
        std::uint64_t stock_directory = 0;
        std::uint64_t add_orders = 0;
        std::uint64_t executions = 0;
        std::uint64_t cancels = 0;
        std::uint64_t deletes = 0;
        std::uint64_t replaces = 0;
        std::uint64_t other = 0;
        std::vector<std::uint8_t> payload;
        market_data_engine::OrderBook book;

        while ((max_messages == 0 || messages < max_messages)
               && market_data_engine::read_binary_file_message(*input, payload)) {
            switch (payload[0]) {
            case 'S':
                (void)market_data_engine::parse_system_event(payload);
                ++system_events;
                break;
            case 'R':
                book.apply(market_data_engine::parse_stock_directory(payload));
                ++stock_directory;
                break;
            case 'A':
                book.apply(market_data_engine::parse_add_order(payload));
                ++add_orders;
                break;
            case 'F':
                book.apply(market_data_engine::parse_add_order_with_mpid(payload));
                ++add_orders;
                break;
            case 'E':
                book.apply(market_data_engine::parse_order_executed(payload));
                ++executions;
                break;
            case 'C':
                book.apply(market_data_engine::parse_order_executed_with_price(payload));
                ++executions;
                break;
            case 'X':
                book.apply(market_data_engine::parse_order_cancel(payload));
                ++cancels;
                break;
            case 'D':
                book.apply(market_data_engine::parse_order_delete(payload));
                ++deletes;
                break;
            case 'U':
                book.apply(market_data_engine::parse_order_replace(payload));
                ++replaces;
                break;
            default:
                ++other;
                break;
            }
            ++messages;
        }

        std::cout << "messages: " << messages << '\n'
                  << "system_events: " << system_events << '\n'
                  << "stock_directory: " << stock_directory << '\n'
                  << "add_orders: " << add_orders << '\n'
                  << "executions: " << executions << '\n'
                  << "cancels: " << cancels << '\n'
                  << "deletes: " << deletes << '\n'
                  << "replaces: " << replaces << '\n'
                  << "other: " << other << '\n'
                  << "active_orders: " << book.active_order_count() << '\n';
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }

    return 0;
}
