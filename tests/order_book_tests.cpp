
#include <cassert>
#include <iostream>
#include <stdexcept>

#include "order_book.hpp"

int main() {
    OrderBook book;

    // Empty book.
    assert(!book.bestBid().has_value());
    assert(!book.bestAsk().has_value());
    assert(book.orderCount() == 0);

    // Multiple orders at the same price.
    book.addOrder(Order{1, Side::Buy, 10000, 10});
    book.addOrder(Order{2, Side::Buy, 10005, 5});
    book.addOrder(Order{3, Side::Buy, 10000, 7});

    book.addOrder(Order{4, Side::Sell, 10010, 8});
    book.addOrder(Order{5, Side::Sell, 10008, 4});
    book.addOrder(Order{6, Side::Sell, 10010, 3});

    assert(book.bestBid().value() == 10005);
    assert(book.bestAsk().value() == 10008);
    assert(book.bidLevelCount() == 2);
    assert(book.askLevelCount() == 2);
    assert(book.orderCount() == 6);

    assert(book.quantityAtBid(10000) == 17);
    assert(book.quantityAtAsk(10010) == 11);

    // A new order creates a better bid and ask.
    book.addOrder(Order{7, Side::Buy, 10012, 6});
    book.addOrder(Order{8, Side::Sell, 10006, 2});

    assert(book.bestBid().value() == 10012);
    assert(book.bestAsk().value() == 10006);

    // Cancel one of two orders at the same bid price.
    assert(book.cancelOrder(1));
    assert(book.quantityAtBid(10000) == 7);
    assert(book.bidLevelCount() == 3);

    // Cancel the remaining order at that price.
    assert(book.cancelOrder(3));
    assert(book.quantityAtBid(10000) == 0);
    assert(book.bidLevelCount() == 2);

    // Unknown IDs must not change the book.
    assert(!book.cancelOrder(999));
    assert(book.orderCount() == 6);

    // Duplicate IDs must be rejected.
    bool duplicateRejected = false;

    try {
        book.addOrder(Order{2, Side::Buy, 9999, 1});
    } catch (const std::invalid_argument&) {
        duplicateRejected = true;
    }

    assert(duplicateRejected);
    assert(book.orderCount() == 6);
    assert(book.bestBid().value() == 10012);

    // Cancel the best bid; the next bid becomes best.
    assert(book.cancelOrder(7));
    assert(book.bestBid().value() == 10005);

    std::cout << "All order book tests passed!\n";


    MatchingEngine fifoEngine;

    fifoEngine.book().addOrder(Order{100, Side::Sell, 20000, 2});
    fifoEngine.book().addOrder(Order{101, Side::Sell, 20000, 3});

    const auto fifoResult = fifoEngine.submitLimitOrder(
        Order{102, Side::Buy, 20000, 3}
    );

    assert(fifoResult.trades.size() == 2);
    assert(fifoResult.trades[0].sellOrderId == 100);
    assert(fifoResult.trades[0].quantity == 2);
    assert(fifoResult.trades[1].sellOrderId == 101);
    assert(fifoResult.trades[1].quantity == 1);
    assert(fifoEngine.book().quantityAtAsk(20000) == 2);

    return 0;
}
