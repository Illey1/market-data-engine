#include "market_data_engine/binary_file.hpp"

#include "big_endian.hpp"

#include <array>
#include <istream>
#include <stdexcept>

namespace market_data_engine {

bool read_binary_file_message(std::istream& input,
                              std::vector<std::uint8_t>& payload) {
    std::array<char, 2> prefix{};
    if (!input.read(prefix.data(), 2)) {
        throw std::runtime_error("Incomplete BinaryFILE length prefix or missing terminator");
    }

    const std::array<std::uint8_t, 2> length_bytes{
        static_cast<std::uint8_t>(prefix[0]),
        static_cast<std::uint8_t>(prefix[1])};
    const auto length = detail::read_be16(length_bytes);
    if (length == 0) {
        payload.clear();
        return false;
    }

    payload.resize(length);
    if (!input.read(static_cast<char*>(static_cast<void*>(payload.data())),
                    static_cast<std::streamsize>(payload.size()))) {
        throw std::runtime_error("Incomplete BinaryFILE payload");
    }
    return true;
}

}
