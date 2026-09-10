#include "market_data_engine/binary_file.hpp"

#include "big_endian.hpp"

#include <array>
#include <istream>
#include <stdexcept>

namespace market_data_engine {

BinaryFileReadResult read_binary_file_message(
    std::istream& input, std::vector<std::uint8_t>& payload) {
    std::array<char, 2> prefix{};
    if (!input.read(prefix.data(), 2)) {
        if (input.gcount() == 0 && input.eof() && !input.bad()) {
            payload.clear();
            return BinaryFileReadResult::end_of_file;
        }
        throw std::runtime_error("Incomplete BinaryFILE length prefix or stream read error");
    }

    const std::array<std::uint8_t, 2> length_bytes{
        static_cast<std::uint8_t>(prefix[0]),
        static_cast<std::uint8_t>(prefix[1])};
    const auto length = detail::read_be16(length_bytes);
    if (length == 0) {
        payload.clear();
        return BinaryFileReadResult::end_of_session;
    }

    payload.resize(length);
    if (!input.read(reinterpret_cast<char*>(payload.data()),
                    static_cast<std::streamsize>(payload.size()))) {
        throw std::runtime_error("Incomplete BinaryFILE payload");
    }
    return BinaryFileReadResult::message;
}

}
