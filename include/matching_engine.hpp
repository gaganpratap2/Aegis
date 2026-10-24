
#pragma once

#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <unordered_set>
#include <vector>

#include "order_book.hpp"

struct Trade {
    std::uint64_t tradeId;
    OrderId buyOrderId;
    OrderId sellOrderId;
    PriceTicks priceTicks;
    Quantity quantity;
};

struct MatchResult {
    std::vector<Trade> trades;
    Quantity remainingQuantity;
    bool resting;
};

class MatchingEngine {
private:
    OrderBook book_;
    std::unordered_set<OrderId> seenOrderIds_;
    std::uint64_t nextTradeId_ = 1;

public:
    MatchResult submitLimitOrder(const Order& incoming) {
        if (incoming.side() != Side::Buy &&
            incoming.side() != Side::Sell) {
            throw std::invalid_argument("Invalid order side");
        }

        // Reject reused IDs, including IDs of fully filled orders.
        if (seenOrderIds_.count(incoming.id()) != 0 ||
            book_.containsOrder(incoming.id())) {
            throw std::invalid_argument("Duplicate order ID");
        }

        seenOrderIds_.insert(incoming.id());

        MatchResult result{
            {},
            incoming.quantity(),
            false
        };

        while (result.remainingQuantity > 0) {
            const auto resting =
                incoming.side() == Side::Buy
                    ? book_.bestAskOrder()
                    : book_.bestBidOrder();

            if (!resting.has_value()) {
                break;
            }

            // Stop when the best opposing price does not cross.
            const bool crosses =
                incoming.side() == Side::Buy
                    ? incoming.priceTicks() >= resting->priceTicks()
                    : incoming.priceTicks() <= resting->priceTicks();

            if (!crosses) {
                break;
            }

            const Quantity executedQuantity = std::min(
                result.remainingQuantity,
                resting->quantity()
            );

            const Trade trade{
                nextTradeId_,
                incoming.side() == Side::Buy
                    ? incoming.id() : resting->id(),
                incoming.side() == Side::Sell
                    ? incoming.id() : resting->id(),
                resting->priceTicks(),
                executedQuantity
            };

            // Update the resting order and remove it if fully filled.
            book_.reduceOrderQuantity(
                resting->id(),
                executedQuantity
            );

            result.trades.push_back(trade);
            ++nextTradeId_;
            result.remainingQuantity -= executedQuantity;
        }

        // Any unmatched remainder becomes a resting limit order.
        if (result.remainingQuantity > 0) {
            book_.addOrder(Order{
                incoming.id(),
                incoming.side(),
                incoming.priceTicks(),
                result.remainingQuantity
            });

            result.resting = true;
        }

        return result;
    }

    const OrderBook& book() const {
        return book_;
    }

    OrderBook& book() {
        return book_;
    }
};
