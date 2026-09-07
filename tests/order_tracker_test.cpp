#include "market_data_engine/order_tracker.hpp"

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
    const auto check_rejected = [&check](const auto& update,
                                         const char* description) {
        try {
            update();
            check(false, description);
        } catch (const std::runtime_error&) {
        } catch (...) {
            check(false, "invalid transition threw the wrong exception type");
        }
    };

    OrderTracker tracker;
    tracker.apply(AddOrderMessage{12, 1, 1000, 100, 'B', 100, "AAPL", 1234500});
    const auto* active = tracker.find(100);
    check(tracker.size() == 1 && active != nullptr, "Add creates an active order");
    check(active != nullptr && active->order_reference == 100 &&
              active->stock_locate == 12 && active->side == 'B' &&
              active->remaining_shares == 100 && active->stock == "AAPL" &&
              active->price_4 == 1234500,
          "Add preserves the displayed order fields");

    tracker.apply(OrderExecutedMessage{12, 2, 1001, 100, 40, 900});
    active = tracker.find(100);
    check(active != nullptr && active->remaining_shares == 60,
          "partial execution reduces remaining shares");
    tracker.apply(OrderExecutedMessage{12, 3, 1002, 100, 60, 901});
    check(tracker.size() == 0 && tracker.find(100) == nullptr,
          "full execution removes the order");

    tracker.apply(AddOrderMessage{12, 4, 1003, 101, 'B', 100, "AAPL", 1234500});
    tracker.apply(OrderCancelMessage{12, 5, 1004, 101, 25});
    active = tracker.find(101);
    check(active != nullptr && active->remaining_shares == 75,
          "partial cancellation reduces remaining shares");
    tracker.apply(OrderDeleteMessage{12, 6, 1005, 101});
    check(tracker.size() == 0 && tracker.find(101) == nullptr,
          "Delete removes all remaining shares");

    tracker.apply(AddOrderWithMpidMessage{
        12, 7, 1006, 200, 'B', 80, "AAPL", 1234500, "AB",
    });
    active = tracker.find(200);
    check(tracker.size() == 1 && active != nullptr &&
              active->order_reference == 200 && active->stock_locate == 12 &&
              active->side == 'B' && active->remaining_shares == 80 &&
              active->stock == "AAPL" && active->price_4 == 1234500,
          "F creates a normal displayed order");
    tracker.apply(OrderExecutedWithPriceMessage{
        12, 8, 1007, 200, 20, 902, 'N', 1234000,
    });
    active = tracker.find(200);
    check(active != nullptr && active->remaining_shares == 60 &&
              active->price_4 == 1234500,
          "non-printable C execution reduces shares and preserves display price");
    tracker.apply(OrderCancelMessage{12, 9, 1008, 200, 60});
    check(tracker.size() == 0 && tracker.find(200) == nullptr,
          "full cancellation removes the order");

    tracker.apply(AddOrderMessage{22, 10, 1009, 300, 'S', 150, "MSFT", 4125000});
    tracker.apply(OrderReplaceMessage{99, 11, 1010, 300, 301, 250, 4100000});
    active = tracker.find(301);
    check(tracker.size() == 1 && tracker.find(300) == nullptr && active != nullptr,
          "Replace moves the order to its new reference");
    check(active != nullptr && active->order_reference == 301 &&
              active->remaining_shares == 250 && active->price_4 == 4100000 &&
              active->stock_locate == 22 && active->side == 'S' &&
              active->stock == "MSFT",
          "Replace changes shares and price while preserving locate, side, stock");
    tracker.apply(OrderExecutedMessage{22, 12, 1011, 301, 25, 903});
    active = tracker.find(301);
    check(active != nullptr && active->remaining_shares == 225,
          "subsequent updates use the replacement reference");

    check_rejected([&tracker] {
        tracker.apply(OrderExecutedMessage{22, 13, 1012, 300, 1, 904});
    }, "the old reference is rejected after replacement");
    check_rejected([&tracker] {
        tracker.apply(OrderExecutedMessage{22, 14, 1013, 301, 226, 905});
    }, "over-execution is rejected");
    check_rejected([&tracker] {
        tracker.apply(OrderCancelMessage{22, 15, 1014, 301, 226});
    }, "over-cancellation is rejected");
    check_rejected([&tracker] {
        tracker.apply(AddOrderMessage{12, 16, 1015, 301, 'B', 1, "AAPL", 1});
    }, "duplicate active Add is rejected");
    check_rejected([&tracker] {
        tracker.apply(OrderDeleteMessage{22, 17, 1016, 999});
    }, "Delete rejects an unknown reference");
    check_rejected([&tracker] {
        tracker.apply(OrderReplaceMessage{22, 18, 1017, 999, 302, 100, 4101000});
    }, "Replace rejects an unknown original reference");
    check_rejected([&tracker] {
        tracker.apply(OrderReplaceMessage{22, 19, 1018, 301, 301, 100, 4101000});
    }, "Replace rejects an already-active new reference, including its own");

    tracker.apply(AddOrderMessage{12, 20, 1019, 400, 'B', 50, "AAPL", 1234500});
    check_rejected([&tracker] {
        tracker.apply(OrderReplaceMessage{22, 21, 1020, 301, 400, 100, 4101000});
    }, "Replace rejects a new reference belonging to another active order");
    active = tracker.find(301);
    const auto* other = tracker.find(400);
    check(tracker.size() == 2 && active != nullptr &&
              active->remaining_shares == 225 && active->price_4 == 4100000 &&
              active->stock_locate == 22 && active->side == 'S' &&
              active->stock == "MSFT" && other != nullptr &&
              other->remaining_shares == 50 && other->price_4 == 1234500 &&
              other->stock == "AAPL" && tracker.find(302) == nullptr,
          "rejected updates leave existing orders intact without adding orders");

    if (failures != 0) {
        std::cerr << failures << " check(s) failed\n";
        return 1;
    }
    std::cout << "All OrderTracker lifecycle checks passed\n";
    return 0;
}
