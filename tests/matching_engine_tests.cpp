
#include <cassert>
#include <iostream>
#include <stdexcept>

#include "matching_engine.hpp"

int main() {
    MatchingEngine engine;

    // Seed the book with resting asks and bids.
    engine.book().addOrder(Order{1, Side::Sell, 10100, 5});
    engine.book().addOrder(Order{2, Side::Sell, 10105, 4});

    engine.book().addOrder(Order{20, Side::Buy, 10010, 2});
    engine.book().addOrder(Order{21, Side::Buy, 10005, 5});

    // Test 1: A buy crosses the best ask and partially fills it.
    const auto first = engine.submitLimitOrder(
        Order{10, Side::Buy, 10102, 3}
    );

    assert(first.trades.size() == 1);
    assert(first.trades[0].priceTicks == 10100);
    assert(first.trades[0].quantity == 3);
    assert(first.trades[0].buyOrderId == 10);
    assert(first.trades[0].sellOrderId == 1);
    assert(first.remainingQuantity == 0);
    assert(!first.resting);
    assert(engine.book().quantityAtAsk(10100) == 2);

    // Test 2: One incoming order matches two price levels.
    const auto second = engine.submitLimitOrder(
        Order{11, Side::Buy, 10105, 5}
    );

    assert(second.trades.size() == 2);

    // Better ask first, then the next ask.
    assert(second.trades[0].priceTicks == 10100);
    assert(second.trades[0].quantity == 2);

    assert(second.trades[1].priceTicks == 10105);
    assert(second.trades[1].quantity == 3);

    assert(second.remainingQuantity == 0);
    assert(engine.book().quantityAtAsk(10100) == 0);
    assert(engine.book().quantityAtAsk(10105) == 1);

    // Test 3: An incoming sell matches the highest bid first.
    const auto third = engine.submitLimitOrder(
        Order{12, Side::Sell, 10005, 4}
    );

    assert(third.trades.size() == 2);
    assert(third.trades[0].priceTicks == 10010);
    assert(third.trades[0].quantity == 2);
    assert(third.trades[1].priceTicks == 10005);
    assert(third.trades[1].quantity == 2);

    assert(third.remainingQuantity == 0);
    assert(engine.book().quantityAtBid(10010) == 0);
    assert(engine.book().quantityAtBid(10005) == 3);

    // Test 4: A non-crossing limit order rests in the book.
    const auto fourth = engine.submitLimitOrder(
        Order{13, Side::Buy, 10000, 6}
    );

    assert(fourth.trades.empty());
    assert(fourth.remainingQuantity == 6);
    assert(fourth.resting);
    assert(engine.book().bestBid().value() == 10005);
    assert(engine.book().quantityAtBid(10000) == 6);

    // Test 5: A previously used ID cannot be reused.
    bool duplicateRejected = false;

    try {
        engine.submitLimitOrder(
            Order{10, Side::Buy, 10105, 1}
        );
    } catch (const std::invalid_argument&) {
        duplicateRejected = true;
    }

    assert(duplicateRejected);
    assert(engine.book().quantityAtBid(10000) == 6);

    std::cout << "All matching engine tests passed!\n";
    return 0;
}
