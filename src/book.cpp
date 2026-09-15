#include <iostream>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cstdint>

#include "../include/readers.h"
#include "../include/messages.h"
#include "../include/orders.h"

using OrderId = std::uint64_t;
using Side = char;
using Quantity = std::int32_t;
using Price = std::int32_t;
using PoolIndex = std::uint32_t;

class OrderBook {
    std::vector<Order> chainPool;
    std::vector<PriceLevelInfo> bidLadder;
    std::vector<PriceLevelInfo> askLadder;
    std::vector<PoolIndex> freeSlots;
    std::unordered_map<OrderId, PoolIndex> orderMap;

    Price minPrice;
    Price maxPrice;
    std::int32_t tickSize;
    std::uint32_t numLevels;

public:
    OrderBook(const StockStats& stockInfo, std::int32_t tickSize)
        : minPrice(stockInfo.minPrice)
        , maxPrice(stockInfo.maxPrice)
        , tickSize(tickSize) {
        numLevels = (stockInfo.maxPrice - stockInfo.minPrice) / tickSize;
        numLevels = numLevels == 0 ? 1 : numLevels;
        // fill ladders with init indexes and set max size for pool
        bidLadder.resize(numLevels, PriceLevelInfo{ UINT32_MAX, UINT32_MAX });
        askLadder.resize(numLevels, PriceLevelInfo{ UINT32_MAX, UINT32_MAX });
        chainPool.reserve(stockInfo.orderCount);
    }

    void addOrder(OrderId id, Side side, Quantity shares, Price price) {
        if (orderMap.contains(id)) {  // duplicate check
            std::cerr << "Order ID already exists in this book.\n";
            return;
        }

        std::uint32_t priceLadderIndex = (price - minPrice) / tickSize;
        PriceLevelInfo& levelInfo = (side == 'B') ? bidLadder[priceLadderIndex] : askLadder[priceLadderIndex];  // start and end of chain at this price

        bool wasEmpty = (levelInfo.head == UINT32_MAX);
        PoolIndex nextIndex, prevIndex;

        if (wasEmpty) {  // not set yet
            prevIndex = nextIndex = UINT32_MAX;
        }
        else {  // has a tail already
            prevIndex = levelInfo.tail;
            nextIndex = UINT32_MAX;
        }

        PoolIndex emptyPoolIndex;

        if (!freeSlots.empty()) {
            emptyPoolIndex = freeSlots.back();  // next open slot
            freeSlots.pop_back();
            chainPool[emptyPoolIndex] = Order{ id, side, shares, price, nextIndex, prevIndex };
        }
        else {  // no slot freed yet (safe since pre-allocated)
            chainPool.push_back(Order{ id, side, shares, price, nextIndex, prevIndex });
            emptyPoolIndex = chainPool.size() - 1;
        }

        if (!wasEmpty) {
            chainPool[levelInfo.tail].nextIndex = emptyPoolIndex;  // update prev tail 
        }

        // update ends of chain
        if (wasEmpty) {
            levelInfo.head = emptyPoolIndex;
        }
        levelInfo.tail = emptyPoolIndex;  // new order is always the new tail

        orderMap[id] = emptyPoolIndex;
    }

    void cancelOrder(OrderId id, Quantity sharesToCancel) {
        auto it = orderMap.find(id);
        if (it == orderMap.end()) {  // order dne
            std::cerr << "Order does not exist in this book.\n";
            return;
        }

        Order& orderToCancel = chainPool[it->second];
        Quantity remainingQuantity = orderToCancel.shares - sharesToCancel;

        if (remainingQuantity < 0) {  // invalid reduction
            std::cerr << "Data is invalid as the shares remaining is negative.\n";
            return;
        }
        else if (remainingQuantity == 0) {  // no shares remain
            deleteOrder(id);
            return;
        }
        else {
            orderToCancel.shares = remainingQuantity;  // cancel share amount
        }
    }
};

int main() {
    return 0;
}