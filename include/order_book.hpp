
#pragma once

#include <cstdint>
#include <functional>
#include <limits>
#include <map>
#include <optional>
#include <stdexcept>

#include "types.hpp"

class OrderBook {
private:
    // Highest bid first.
    std::map<PriceTicks, Quantity, std::greater<PriceTicks>> bids_;

    // Lowest ask first.
    std::map<PriceTicks, Quantity> asks_;

    template <typename PriceMap>
    static void addToLevels(
        PriceMap& levels,
        PriceTicks price,
        Quantity quantity
    ) {
        auto it = levels.find(price);

        if (it == levels.end()) {
            levels.emplace(price, quantity);
            return;
        }

        // Prevent unsigned quantity overflow.
        if (quantity >
            std::numeric_limits<Quantity>::max() - it->second) {
            throw std::overflow_error("Order book quantity overflow");
        }

        it->second += quantity;
    }

public:
    void addOrder(const Order& order) {
        if (order.side() == Side::Buy) {
            addToLevels(bids_, order.priceTicks(), order.quantity());
        } else {
            addToLevels(asks_, order.priceTicks(), order.quantity());
        }
    }

    std::optional<PriceTicks> bestBid() const {
        if (bids_.empty()) {
            return std::nullopt;
        }

        return bids_.begin()->first;
    }

    std::optional<PriceTicks> bestAsk() const {
        if (asks_.empty()) {
            return std::nullopt;
        }

        return asks_.begin()->first;
    }

    std::size_t bidLevelCount() const {
        return bids_.size();
    }

    std::size_t askLevelCount() const {
        return asks_.size();
    }
};
