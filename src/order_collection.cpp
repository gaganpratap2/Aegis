
#include <iostream>
#include <vector>
#include "types.hpp"

int main() {
    std::vector<Order> orders;

    orders.push_back(Order{101, Side::Buy, 10005, 10});
    orders.push_back(Order{102, Side::Sell, 10010, 20});
    orders.push_back(Order{103, Side::Buy, 10000, 5});

    std::cout << "Total orders: " << orders.size() << '\n';

    for (const Order& order : orders) {
        std::cout << "ID: " << order.id()
                  << ", Side: "
                  << (order.side() == Side::Buy ? "BUY" : "SELL")
                  << ", Price: " << order.priceTicks()
                  << ", Quantity: " << order.quantity()
                  << '\n';
    }

    return 0;
}
