
#include <cassert>
#include <iostream>
#include <stdexcept>
#include "types.hpp"

int main() {
    // A valid order should be created successfully.
    Order validOrder{1, Side::Buy, 10005, 10};
    assert(validOrder.isValid());

    // Invalid orders should throw exceptions.
    bool rejectedZeroId = false;

    try {
        Order order{0, Side::Buy, 10005, 10};
    } catch (const std::invalid_argument&) {
        rejectedZeroId = true;
    }

    assert(rejectedZeroId);

    bool rejectedZeroPrice = false;

    try {
        Order order{2, Side::Buy, 0, 10};
    } catch (const std::invalid_argument&) {
        rejectedZeroPrice = true;
    }

    assert(rejectedZeroPrice);

    bool rejectedZeroQuantity = false;

    try {
        Order order{3, Side::Sell, 10005, 0};
    } catch (const std::invalid_argument&) {
        rejectedZeroQuantity = true;
    }

    assert(rejectedZeroQuantity);


    std::cout << "All constructor tests passed!\n";

    return 0;
}
