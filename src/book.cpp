#include <iostream>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cstdint>
#include <optional>

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

    Price minPrice, maxPrice;
    std::int32_t tickSize;
    std::uint32_t numLevels;

    void reduceShares(Order& updatingOrder, Quantity sharesToReduce) {
        Quantity remainingQuantity = updatingOrder.shares - sharesToReduce;

        if (remainingQuantity < 0) {  // invalid reduction
            std::cerr << "Data is invalid as the shares remaining is negative.\n";
            return;
        }
        else if (remainingQuantity == 0) {  // no shares remain
            deleteOrder(updatingOrder.id);
            return;
        }
        else {
            updatingOrder.shares = remainingQuantity;  // perform share reduction
        }
    }

public:
    OrderBook(const StockStats& stockInfo, std::int32_t tickSize = 1)
        : minPrice(stockInfo.minPrice)
        , maxPrice(stockInfo.maxPrice)
        , tickSize(tickSize) {
        numLevels = ((stockInfo.maxPrice - stockInfo.minPrice) / tickSize) + 1;
        numLevels = numLevels == 0 ? 1 : numLevels;
        // fill ladders with init indexes and set max size for pool
        bidLadder.resize(numLevels, PriceLevelInfo{ UINT32_MAX, UINT32_MAX });
        askLadder.resize(numLevels, PriceLevelInfo{ UINT32_MAX, UINT32_MAX });
        chainPool.reserve(stockInfo.orderCount);
    }

    std::optional<Price> getBestBid() {  // highest price willing to buy at
        for (std::uint32_t i = bidLadder.size(); i > 0; i--) {
            if (bidLadder[i - 1].head != UINT32_MAX) {  // first populated order
                return chainPool[bidLadder[i - 1].head].price;
            }
        }
        return std::nullopt;
    }

    std::optional<Price> getBestAsk() {  // lowest price willing to sell at
        for (std::uint32_t i = 0; i < askLadder.size(); i++) {
            if (askLadder[i].head != UINT32_MAX) {  // first populated order
                return chainPool[askLadder[i].head].price;
            }
        }
        return std::nullopt;
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
        reduceShares(chainPool[it->second], sharesToCancel);
    }

    void deleteOrder(OrderId id) {
        auto it = orderMap.find(id);
        if (it == orderMap.end()) {  // order dne
            std::cerr << "Order does not exist in this book.\n";
            return;
        }

        Order& orderToDelete = chainPool[it->second];
        std::uint32_t priceLadderIndex = (orderToDelete.price - minPrice) / tickSize;
        PriceLevelInfo& levelInfo = (orderToDelete.side == 'B') ? bidLadder[priceLadderIndex] : askLadder[priceLadderIndex];  // chain bound indexes

        // find role of order in the chain
        bool isHead = (levelInfo.head == it->second);
        bool isTail = (levelInfo.tail == it->second);

        // handle deletion cases with chain bounds
        if (isHead && isTail) {  // only order
            levelInfo.head = levelInfo.tail = UINT32_MAX;
        }
        else if (isHead && !isTail) {  // front of chain
            levelInfo.head = chainPool[levelInfo.head].nextIndex;
            chainPool[levelInfo.head].prevIndex = UINT32_MAX;
        }
        else if (!isHead && isTail) {  // back of chain
            levelInfo.tail = chainPool[levelInfo.tail].prevIndex;
            chainPool[levelInfo.tail].nextIndex = UINT32_MAX;
        }
        else {  // middle of chain (skip over)
            chainPool[orderToDelete.prevIndex].nextIndex = orderToDelete.nextIndex;
            chainPool[orderToDelete.nextIndex].prevIndex = orderToDelete.prevIndex;
        }

        freeSlots.push_back(it->second);  // free up slot
        orderMap.erase(id);  // delete
    }

    void replaceOrder(OrderId initialId, OrderId newId, Quantity newShares, Price newPrice) {
        auto it = orderMap.find(initialId);
        if (it == orderMap.end()) {  // order dne
            std::cerr << "Order does not exist in this book.\n";
            return;
        }
        if (orderMap.contains(newId)) {  // duplicate check
            std::cerr << "Order ID already exists in this book.\n";
            return;
        }
        Side initialSide = chainPool[it->second].side;

        // delete old and add updated order
        deleteOrder(initialId);
        addOrder(newId, initialSide, newShares, newPrice);
    }

    void executedOrder(OrderId id, Quantity sharesToExecute) {
        auto it = orderMap.find(id);
        if (it == orderMap.end()) {  // order dne
            std::cerr << "Order does not exist in this book.\n";
            return;
        }
        reduceShares(chainPool[it->second], sharesToExecute);
    }
};