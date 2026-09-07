#include "market_data_engine/add_order.hpp"
#include "market_data_engine/binary_file.hpp"
#include "market_data_engine/modify_order.hpp"
#include "market_data_engine/order_book.hpp"
#include "market_data_engine/system_event.hpp"

#include <cstdint>
#include <exception>
#include <fstream>
#include <iostream>
#include <vector>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <decompressed-binaryfile>\n";
        return 1;
    }

    try {
        std::ifstream input(argv[1], std::ios::binary);
        if (!input) {
            std::cerr << "Cannot open file: " << argv[1] << '\n';
            return 1;
        }

        std::uint64_t messages = 0;
        std::uint64_t system_events = 0;
        std::uint64_t add_orders = 0;
        std::uint64_t executions = 0;
        std::uint64_t cancels = 0;
        std::uint64_t deletes = 0;
        std::uint64_t replaces = 0;
        std::uint64_t other = 0;
        std::vector<std::uint8_t> payload;
        market_data_engine::OrderBook book;

        while (market_data_engine::read_binary_file_message(input, payload)) {
            switch (payload[0]) {
            case 'S':
                (void)market_data_engine::parse_system_event(payload);
                ++system_events;
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
