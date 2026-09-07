#include "market_data_engine/order_book.hpp"

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

    OrderBook book;
    check(!book.best_bid("AAPL") && !book.best_ask("AAPL") &&
              book.quantity_at("AAPL", 'B', 1234500) == 0 &&
              book.active_order_count() == 0,
          "an unknown symbol has no levels or active orders");

    book.apply(AddOrderMessage{12, 1, 1000, 100, 'B', 100, "AAPL", 1234500});
    check(is_level(book.best_bid("AAPL"), 1234500, 100) &&
              !book.best_ask("AAPL") && book.active_order_count() == 1,
          "a buy order creates a bid level");
    book.apply(AddOrderMessage{12, 2, 1001, 101, 'S', 80, "AAPL", 1235500});
    check(is_level(book.best_ask("AAPL"), 1235500, 80) &&
              book.active_order_count() == 2,
          "a sell order creates an ask level");
    book.apply(AddOrderWithMpidMessage{
        12, 3, 1002, 102, 'B', 60, "AAPL", 1234500, "ABCD",
    });
    check(is_level(book.best_bid("AAPL"), 1234500, 160) &&
              book.quantity_at("AAPL", 'B', 1234500) == 160 &&
              book.active_order_count() == 3,
          "A and F orders at the same price aggregate their shares");

    book.apply(AddOrderMessage{12, 4, 1003, 103, 'B', 40, "AAPL", 1235000});
    book.apply(AddOrderMessage{12, 5, 1004, 104, 'S', 70, "AAPL", 1235200});
    check(is_level(book.best_bid("AAPL"), 1235000, 40) &&
              is_level(book.best_ask("AAPL"), 1235200, 70) &&
              book.quantity_at("AAPL", 'S', 1235500) == 80,
          "higher bids and lower asks become the best levels");

    book.apply(OrderExecutedMessage{12, 6, 1005, 100, 30, 900});
    check(book.quantity_at("AAPL", 'B', 1234500) == 130 &&
              is_level(book.best_bid("AAPL"), 1235000, 40),
          "partial execution reduces the referenced order's shared level");
    book.apply(OrderCancelMessage{12, 7, 1006, 102, 20});
    check(book.quantity_at("AAPL", 'B', 1234500) == 110 &&
              book.active_order_count() == 5,
          "partial cancellation reduces aggregate shares and retains the order");
    book.apply(OrderExecutedMessage{12, 8, 1007, 103, 40, 901});
    book.apply(OrderDeleteMessage{12, 9, 1008, 104});
    check(book.quantity_at("AAPL", 'B', 1235000) == 0 &&
              book.quantity_at("AAPL", 'S', 1235200) == 0 &&
              is_level(book.best_bid("AAPL"), 1234500, 110) &&
              is_level(book.best_ask("AAPL"), 1235500, 80) &&
              book.active_order_count() == 3,
          "full execution and delete erase empty levels and reveal the next best");

    book.apply(AddOrderMessage{12, 10, 1009, 105, 'B', 40, "AAPL", 1234000});
    book.apply(OrderExecutedWithPriceMessage{
        12, 11, 1010, 100, 20, 902, 'N', 1234000,
    });
    check(book.quantity_at("AAPL", 'B', 1234500) == 90 &&
              book.quantity_at("AAPL", 'B', 1234000) == 40,
          "C reduces the original displayed price, leaving the execution-price level intact");

    book.apply(AddOrderMessage{22, 12, 1011, 200, 'B', 300, "MSFT", 1234500});
    book.apply(AddOrderMessage{22, 13, 1012, 201, 'S', 60, "MSFT", 1235500});
    check(is_level(book.best_bid("MSFT"), 1234500, 300) &&
              is_level(book.best_ask("MSFT"), 1235500, 60) &&
              is_level(book.best_bid("AAPL"), 1234500, 90) &&
              is_level(book.best_ask("AAPL"), 1235500, 80),
          "symbols with the same prices maintain independent quantities");

    book.apply(OrderReplaceMessage{12, 14, 1013, 100, 106, 120, 1234800});
    check(book.quantity_at("AAPL", 'B', 1234500) == 40 &&
              is_level(book.best_bid("AAPL"), 1234800, 120) &&
              book.active_order_count() == 6,
          "Replace removes only the original remaining shares and creates a new level");
    book.apply(OrderReplaceMessage{12, 15, 1014, 106, 107, 25, 1234500});
    check(book.quantity_at("AAPL", 'B', 1234800) == 0 &&
              is_level(book.best_bid("AAPL"), 1234500, 65) &&
              book.active_order_count() == 6,
          "Replace erases an emptied old level and adds to an existing new level");

    book.apply(OrderDeleteMessage{12, 16, 1015, 107});
    check(book.quantity_at("AAPL", 'B', 1234500) == 40,
          "deleting a replacement preserves the other order at its level");
    book.apply(OrderCancelMessage{12, 17, 1016, 102, 40});
    book.apply(OrderDeleteMessage{12, 18, 1017, 105});
    check(!book.best_bid("AAPL") &&
              book.quantity_at("AAPL", 'B', 1234500) == 0 &&
              is_level(book.best_ask("AAPL"), 1235500, 80),
          "removing the last bids leaves no bid and preserves the asks");
    book.apply(OrderDeleteMessage{12, 19, 1018, 101});
    check(!book.best_bid("AAPL") && !book.best_ask("AAPL") &&
              is_level(book.best_bid("MSFT"), 1234500, 300) &&
              is_level(book.best_ask("MSFT"), 1235500, 60) &&
              book.active_order_count() == 2,
          "emptying one symbol preserves the other symbol's book");

    const auto check_rejected = [&book, &check, &is_level](
                                    const auto& update, const char* description) {
        try {
            update();
            check(false, description);
        } catch (const std::runtime_error&) {
        } catch (...) {
            check(false, "invalid transition threw the wrong exception type");
        }
        check(book.active_order_count() == 2 &&
                  is_level(book.best_bid("MSFT"), 1234500, 300) &&
                  is_level(book.best_ask("MSFT"), 1235500, 60) &&
                  book.quantity_at("MSFT", 'B', 1234400) == 0 &&
                  !book.best_bid("AAPL") && !book.best_ask("AAPL"),
              "rejected updates leave aggregate levels and active count unchanged");
    };
    check_rejected([&book] {
        book.apply(AddOrderMessage{22, 20, 1019, 200, 'B', 10, "MSFT", 1234400});
    }, "duplicate Add is rejected");
    check_rejected([&book] {
        book.apply(OrderDeleteMessage{22, 21, 1020, 999});
    }, "Delete rejects an unknown reference");
    check_rejected([&book] {
        book.apply(OrderExecutedMessage{22, 22, 1021, 200, 301, 903});
    }, "over-execution is rejected");
    check_rejected([&book] {
        book.apply(OrderCancelMessage{22, 23, 1022, 201, 61});
    }, "over-cancellation is rejected");
    check_rejected([&book] {
        book.apply(OrderReplaceMessage{22, 24, 1023, 200, 201, 100, 1234400});
    }, "replacement reference collision is rejected");
    book.apply(OrderExecutedMessage{22, 25, 1024, 200, 100, 904});
    check(is_level(book.best_bid("MSFT"), 1234500, 200) &&
              book.active_order_count() == 2,
          "the original order remains usable after rejected updates");

    OrderBook large_level;
    large_level.apply(AddOrderMessage{
        32, 1, 2000, 300, 'B', 3000000000U, "LARGE", 10000,
    });
    large_level.apply(AddOrderMessage{
        32, 2, 2001, 301, 'B', 2000000000U, "LARGE", 10000,
    });
    check(is_level(large_level.best_bid("LARGE"), 10000, 5000000000ULL) &&
              large_level.quantity_at("LARGE", 'B', 10000) == 5000000000ULL,
          "aggregate shares can exceed the range of an individual order quantity");
    large_level.apply(OrderExecutedMessage{32, 3, 2002, 300, 1000000000U, 905});
    check(is_level(large_level.best_bid("LARGE"), 10000, 4000000000ULL) &&
              large_level.active_order_count() == 2,
          "execution subtracts correctly from a 64-bit aggregate");

    if (failures != 0) {
        std::cerr << failures << " check(s) failed\n";
        return 1;
    }
    std::cout << "All OrderBook lifecycle checks passed\n";
    return 0;
}
