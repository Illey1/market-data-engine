#include "market_data_engine/binary_file.hpp"

#include <cstdint>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

int main() {
    using market_data_engine::BinaryFileReadResult;

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
            while (market_data_engine::read_binary_file_message(input, payload)
                   == BinaryFileReadResult::message) {
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

    check(market_data_engine::read_binary_file_message(input, payload)
              == BinaryFileReadResult::message,
          "first framed message is read");
    check(payload == std::vector<std::uint8_t>{'S', 'x', 'y'},
          "first payload matches its frame");
    check(market_data_engine::read_binary_file_message(input, payload)
              == BinaryFileReadResult::message,
          "second framed message is read");
    check(payload == std::vector<std::uint8_t>(258, 'A'),
          "two-byte big-endian length and second payload are decoded");
    check(market_data_engine::read_binary_file_message(input, payload)
              == BinaryFileReadResult::end_of_session,
          "zero-length record terminates the stream");
    check(payload.empty(), "terminator clears the previous payload");

    std::istringstream unterminated(std::string{"\x00\x01S", 3}, std::ios::binary);
    check(market_data_engine::read_binary_file_message(unterminated, payload)
              == BinaryFileReadResult::message,
          "a complete message before EOF is read");
    check(payload == std::vector<std::uint8_t>{'S'}, "last complete payload is retained");
    check(market_data_engine::read_binary_file_message(unterminated, payload)
              == BinaryFileReadResult::end_of_file,
          "clean EOF after a complete message is distinct from a terminator");
    check(payload.empty(), "clean EOF clears the previous payload");

    check_invalid(std::string{"\x00", 1}, "truncated length prefix is rejected");
    check_invalid(std::string{"\x00\x03Sx", 4}, "truncated payload is rejected");

    std::istringstream failed_input;
    failed_input.setstate(std::ios::badbit | std::ios::eofbit);
    try {
        (void)market_data_engine::read_binary_file_message(failed_input, payload);
        check(false, "a stream error must not be treated as clean EOF");
    } catch (const std::runtime_error&) {
    } catch (...) {
        check(false, "stream error threw the wrong exception type");
    }

    if (failures != 0) {
        std::cerr << failures << " check(s) failed\n";
        return 1;
    }
    std::cout << "All BinaryFILE reader checks passed\n";
    return 0;
}
