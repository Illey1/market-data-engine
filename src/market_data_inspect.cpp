#include "market_data_engine/add_order.hpp"
#include "market_data_engine/binary_file.hpp"
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
        std::uint64_t other = 0;
        std::vector<std::uint8_t> payload;

        while (market_data_engine::read_binary_file_message(input, payload)) {
            switch (payload[0]) {
            case 'S':
                (void)market_data_engine::parse_system_event(payload);
                ++system_events;
                break;
            case 'A':
                (void)market_data_engine::parse_add_order(payload);
                ++add_orders;
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
                  << "other: " << other << '\n';
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }

    return 0;
}
