
#include <iostream>
#include "types.hpp"

int main() {
    Order order{1, Side::Buy, 10005, 10};

    std::cout << "Aegis HFT Simulator Starting...\n";
    std::cout << "Order ID: " << order.id() << '\n';
    std::cout << "Price ticks: " << order.priceTicks() << '\n';
    std::cout << "Quantity: " << order.quantity() << '\n';

    std::cout << "Side: "
              << (order.side() == Side::Buy ? "BUY" : "SELL")
              << '\n';

    return 0;
}
