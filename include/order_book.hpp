
#pragma once

#include <cstddef>
#include <deque>
#include <functional>
#include <map>
#include <optional>
#include <stdexcept>
#include <unordered_map>

#include "types.hpp"

class OrderBook {
private:
    using OrderQueue = std::deque<Order>;

    std::map<PriceTicks, OrderQueue, std::greater<PriceTicks>> bids_;
    std::map<PriceTicks, OrderQueue> asks_;

    struct Location {
        Side side;
        PriceTicks price;
    };

    std::unordered_map<OrderId, Location> locations_;

public:
    void addOrder(const Order& order) {
        if (order.side() != Side::Buy &&
            order.side() != Side::Sell) {
            throw std::invalid_argument("Invalid order side");
        }

        if (locations_.find(order.id()) != locations_.end()) {
            throw std::invalid_argument("Duplicate order ID");
        }

        if (order.side() == Side::Buy) {
            bids_[order.priceTicks()].push_back(order);
        } else {
            asks_[order.priceTicks()].push_back(order);
        }

        try {
            locations_.emplace(
                order.id(),
                Location{order.side(), order.priceTicks()}
            );
        } catch (...) {
            // Roll back the queue change if indexing fails.
            if (order.side() == Side::Buy) {
                auto level = bids_.find(order.priceTicks());
                level->second.pop_back();

                if (level->second.empty()) {
                    bids_.erase(level);
                }
            } else {
                auto level = asks_.find(order.priceTicks());
                level->second.pop_back();

                if (level->second.empty()) {
                    asks_.erase(level);
                }
            }

            throw;
        }
    }

    // Returns true if the order existed and was removed.
    // Returns false if the ID was not found.
    bool cancelOrder(OrderId id) {
        auto location = locations_.find(id);

        if (location == locations_.end()) {
            return false;
        }

        const Side side = location->second.side;
        const PriceTicks price = location->second.price;

        if (side == Side::Buy) {
            auto level = bids_.find(price);

            for (auto order = level->second.begin();
                 order != level->second.end(); ++order) {
                if (order->id() == id) {
                    level->second.erase(order);
                    break;
                }
            }

            if (level->second.empty()) {
                bids_.erase(level);
            }
        } else {
            auto level = asks_.find(price);

            for (auto order = level->second.begin();
                 order != level->second.end(); ++order) {
                if (order->id() == id) {
                    level->second.erase(order);
                    break;
                }
            }

            if (level->second.empty()) {
                asks_.erase(level);
            }
        }

        locations_.erase(location);
        return true;
    }

    bool containsOrder(OrderId id) const {
        return locations_.find(id) != locations_.end();
    }

    std::optional<Order> bestBidOrder() const {
        if (bids_.empty()) {
            return std::nullopt;
        }

        return bids_.begin()->second.front();
    }

    std::optional<Order> bestAskOrder() const {
        if (asks_.empty()) {
            return std::nullopt;
        }

        return asks_.begin()->second.front();
    }

    bool reduceOrderQuantity(OrderId id, Quantity executedQuantity) {
        auto location = locations_.find(id);

        if (location == locations_.end()) {
            return false;
        }

        if (executedQuantity == 0) {
            throw std::invalid_argument(
                "Executed quantity must be positive"
            );
        }

        const Side side = location->second.side;
        const PriceTicks price = location->second.price;

        if (side == Side::Buy) {
            auto level = bids_.find(price);

            for (auto it = level->second.begin();
                 it != level->second.end(); ++it) {
                if (it->id() == id) {
                    if (executedQuantity > it->quantity()) {
                        throw std::invalid_argument(
                            "Executed quantity exceeds remaining quantity"
                        );
                    }

                    if (executedQuantity == it->quantity()) {
                        cancelOrder(id);
                    } else {
                        *it = Order{
                            it->id(),
                            it->side(),
                            it->priceTicks(),
                            it->quantity() - executedQuantity
                        };
                    }

                    return true;
                }
            }
        } else {
            auto level = asks_.find(price);

            for (auto it = level->second.begin();
                 it != level->second.end(); ++it) {
                if (it->id() == id) {
                    if (executedQuantity > it->quantity()) {
                        throw std::invalid_argument(
                            "Executed quantity exceeds remaining quantity"
                        );
                    }

                    if (executedQuantity == it->quantity()) {
                        cancelOrder(id);
                    } else {
                        *it = Order{
                            it->id(),
                            it->side(),
                            it->priceTicks(),
                            it->quantity() - executedQuantity
                        };
                    }

                    return true;
                }
            }
        }

        throw std::logic_error(
            "Order index and price-level queue are inconsistent"
        );
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

    std::size_t orderCount() const {
        return locations_.size();
    }

    Quantity quantityAtBid(PriceTicks price) const {
        auto level = bids_.find(price);

        if (level == bids_.end()) {
            return 0;
        }

        Quantity total = 0;

        for (const Order& order : level->second) {
            total += order.quantity();
        }

        return total;
    }

    Quantity quantityAtAsk(PriceTicks price) const {
        auto level = asks_.find(price);

        if (level == asks_.end()) {
            return 0;
        }

        Quantity total = 0;

        for (const Order& order : level->second) {
            total += order.quantity();
        }

        return total;
    }
};
