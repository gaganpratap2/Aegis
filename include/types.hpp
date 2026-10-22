
#pragma once

#include <cstdint>
#include <stdexcept>

using OrderId = std::uint64_t;
using PriceTicks = std::int64_t;
using Quantity = std::uint64_t;

enum class Side {
    Buy,
    Sell
};

class Order {
private:
    OrderId id_;
    Side side_;
    PriceTicks priceTicks_;
    Quantity quantity_;

public:
    Order(
        OrderId id,
        Side side,
        PriceTicks priceTicks,
        Quantity quantity
    )
        : id_(id),
          side_(side),
          priceTicks_(priceTicks),
          quantity_(quantity)
    {
        if (id_ == 0) {
            throw std::invalid_argument("Order ID must be positive");
        }

        if (priceTicks_ <= 0) {
            throw std::invalid_argument("Price must be positive");
        }

        if (quantity_ == 0) {
            throw std::invalid_argument("Quantity must be positive");
        }
    }

    OrderId id() const {
        return id_;
    }

    Side side() const {
        return side_;
    }

    PriceTicks priceTicks() const {
        return priceTicks_;
    }

    Quantity quantity() const {
        return quantity_;
    }
};
