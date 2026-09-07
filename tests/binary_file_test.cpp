#include "market_data_engine/binary_file.hpp"

#include <cstdint>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

int main() {
    int failures = 0;
    const auto check = [&failures](bool condition, const char* description) {
        if (!condition) {
            std::cerr << "FAIL: " << description << '\n';
            ++failures;
        }
    };
    const auto check_invalid = [&check](const std::string& bytes,
                                        const char* description) {
        std::istringstream input(bytes, std::ios::binary);
        std::vector<std::uint8_t> payload;
        try {
            while (market_data_engine::read_binary_file_message(input, payload)) {
            }
            check(false, description);
        } catch (const std::runtime_error&) {
        } catch (...) {
            check(false, "malformed stream threw the wrong exception type");
        }
    };

    std::string bytes{"\x00\x03", 2};
    bytes += "Sxy";
    bytes.append("\x01\x02", 2);
    bytes.append(258, 'A');
    bytes.append("\x00\x00", 2);
    std::istringstream input(bytes, std::ios::binary);
    std::vector<std::uint8_t> payload;

    check(market_data_engine::read_binary_file_message(input, payload),
          "first framed message is read");
    check(payload == std::vector<std::uint8_t>{'S', 'x', 'y'},
          "first payload matches its frame");
    check(market_data_engine::read_binary_file_message(input, payload),
          "second framed message is read");
    check(payload == std::vector<std::uint8_t>(258, 'A'),
          "two-byte big-endian length and second payload are decoded");
    check(!market_data_engine::read_binary_file_message(input, payload),
          "zero-length record terminates the stream");
    check(payload.empty(), "terminator clears the previous payload");

    check_invalid(std::string{"\x00", 1}, "truncated length prefix is rejected");
    check_invalid(std::string{"\x00\x03Sx", 4}, "truncated payload is rejected");
    check_invalid(std::string{"\x00\x01S", 3}, "missing terminator is rejected");

    if (failures != 0) {
        std::cerr << failures << " check(s) failed\n";
        return 1;
    }
    std::cout << "All BinaryFILE reader checks passed\n";
    return 0;
}
