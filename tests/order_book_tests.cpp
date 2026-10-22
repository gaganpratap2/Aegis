
#include <cassert>
#include <iostream>

#include "order_book.hpp"

int main() {
    OrderBook book;

    // Test 1: An empty book has no best prices.
    assert(!book.bestBid().has_value());
    assert(!book.bestAsk().has_value());

    // Test 2: Add buy and sell orders.
    book.addOrder(Order{1, Side::Buy, 10000, 10});
    book.addOrder(Order{2, Side::Buy, 10005, 5});
    book.addOrder(Order{3, Side::Buy, 10000, 7});

    book.addOrder(Order{4, Side::Sell, 10010, 8});
    book.addOrder(Order{5, Side::Sell, 10008, 4});
    book.addOrder(Order{6, Side::Sell, 10010, 3});

    // Test 3: Best prices must follow the correct sorting.
    assert(book.bestBid().value() == 10005);
    assert(book.bestAsk().value() == 10008);

    // Test 4: Orders at the same price share a price level.
    assert(book.bidLevelCount() == 2);
    assert(book.askLevelCount() == 2);

    std::cout << "All order book tests passed!\n";

    return 0;
}
