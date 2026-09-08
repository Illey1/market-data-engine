#include "market_data_engine/order_book.hpp"
#include "market_data_engine/stock_directory.hpp"

#include <cstdint>
#include <iostream>
#include <stdexcept>

int main() {
    using namespace market_data_engine;

    int failures = 0;
    const auto check = [&failures](bool condition, const char* description) {
        if (!condition) {
            std::cerr << "FAIL: " << description << '\n';
            ++failures;
        }
    };
    const auto is_level = [](const auto& level, std::uint32_t price_4,
                             std::uint64_t shares) {
        return level.has_value() && level->price_4 == price_4 &&
               level->shares == shares;
    };

    StockDirectoryMessage directory{};
    directory.stock_locate = 12;
    directory.tracking_number = 1;
    directory.timestamp_ns = 1000;
    directory.stock = "AAPL";

    OrderBook book;
    book.apply(directory);
    book.apply(AddOrderMessage{12, 2, 1001, 100, 'B', 100, "AAPL", 10000});
    book.apply(AddOrderWithMpidMessage{
        12, 3, 1002, 101, 'S', 80, "AAPL", 11000, "ABCD",
    });
    check(is_level(book.best_bid("AAPL"), 10000, 100) &&
              is_level(book.best_ask("AAPL"), 11000, 80) &&
              book.active_order_count() == 2,
          "A and F matching a Stock Directory entry create their levels");

    const auto check_rejected = [&book, &check, &is_level](
                                    const auto& message, const char* description) {
        try {
            book.apply(message);
            check(false, description);
        } catch (const std::runtime_error&) {
        } catch (...) {
            check(false, "inconsistent message threw the wrong exception type");
        }
        check(book.active_order_count() == 2 &&
                  is_level(book.best_bid("AAPL"), 10000, 100) &&
                  is_level(book.best_ask("AAPL"), 11000, 80) &&
                  book.quantity_at("AAPL", 'B', 9900) == 0 &&
                  !book.best_bid("MSFT") && !book.best_ask("MSFT"),
              "rejected consistency checks preserve aggregate levels and active count");
    };

    check_rejected(AddOrderMessage{12, 4, 1003, 102, 'B', 20, "MSFT", 10000},
                   "A rejects a symbol that differs from its directory entry");
    check_rejected(AddOrderWithMpidMessage{
        12, 5, 1004, 102, 'S', 20, "MSFT", 11000, "ABCD",
    }, "F rejects a symbol that differs from its directory entry");
    check_rejected(AddOrderMessage{12, 6, 1005, 102, 'X', 20, "AAPL", 10000},
                   "A rejects an invalid side before insertion");
    check_rejected(AddOrderWithMpidMessage{
        12, 7, 1006, 102, '?', 20, "AAPL", 11000, "ABCD",
    }, "F rejects an invalid side before insertion");

    check_rejected(OrderExecutedMessage{13, 8, 1007, 100, 25, 900},
                   "E rejects a locate that differs from the active order");
    check_rejected(OrderExecutedWithPriceMessage{
        13, 9, 1008, 100, 25, 901, 'Y', 9900,
    }, "C rejects a locate that differs from the active order");
    check_rejected(OrderCancelMessage{13, 10, 1009, 100, 25},
                   "X rejects a locate that differs from the active order");
    check_rejected(OrderDeleteMessage{13, 11, 1010, 100},
                   "D rejects a locate that differs from the active order");
    check_rejected(OrderReplaceMessage{13, 12, 1011, 100, 103, 50, 9900},
                   "U rejects a locate that differs from the original order");

    check_rejected(directory, "an exact duplicate directory locate is rejected");
    StockDirectoryMessage conflict = directory;
    conflict.stock = "MSFT";
    check_rejected(conflict, "a conflicting directory locate is rejected");
    check_rejected(AddOrderMessage{12, 13, 1012, 102, 'B', 20, "MSFT", 10000},
                   "a rejected conflicting directory entry does not overwrite the mapping");

    book.apply(AddOrderMessage{12, 14, 1013, 102, 'B', 20, "AAPL", 10000});
    check(book.quantity_at("AAPL", 'B', 10000) == 120 &&
              book.active_order_count() == 3,
          "the original mapping and rejected Add reference remain usable");
    book.apply(OrderExecutedMessage{12, 15, 1014, 100, 25, 902});
    check(book.quantity_at("AAPL", 'B', 10000) == 95 &&
              book.active_order_count() == 3,
          "valid execution still finds the original order after rejected updates");
    book.apply(OrderReplaceMessage{12, 16, 1015, 100, 103, 50, 9900});
    check(book.quantity_at("AAPL", 'B', 10000) == 20 &&
              book.quantity_at("AAPL", 'B', 9900) == 50 &&
              is_level(book.best_ask("AAPL"), 11000, 80) &&
              book.active_order_count() == 3,
          "valid replacement removes the original remaining shares and preserves stock and side");
    book.apply(OrderDeleteMessage{12, 17, 1016, 103});
    check(book.quantity_at("AAPL", 'B', 9900) == 0 &&
              book.active_order_count() == 2,
          "the replacement retains the original locate");

    OrderBook without_directory;
    without_directory.apply(AddOrderMessage{
        22, 1, 2000, 200, 'B', 10, "AAPL", 10000,
    });
    without_directory.apply(AddOrderWithMpidMessage{
        22, 2, 2001, 201, 'S', 20, "MSFT", 20000, "ABCD",
    });
    check(without_directory.quantity_at("AAPL", 'B', 10000) == 10 &&
              without_directory.quantity_at("MSFT", 'S', 20000) == 20 &&
              without_directory.active_order_count() == 2,
          "A and F without a directory entry succeed without inventing a mapping");
    directory.stock_locate = 22;
    directory.stock = "MSFT";
    without_directory.apply(directory);
    without_directory.apply(AddOrderMessage{
        22, 3, 2002, 202, 'B', 30, "MSFT", 19000,
    });
    check(without_directory.quantity_at("MSFT", 'B', 19000) == 30 &&
              without_directory.quantity_at("AAPL", 'B', 10000) == 10 &&
              without_directory.active_order_count() == 3,
          "a later directory entry governs subsequent Adds without changing existing orders");

    if (failures != 0) {
        std::cerr << failures << " check(s) failed\n";
        return 1;
    }
    std::cout << "All stock-locate consistency checks passed\n";
    return 0;
}
