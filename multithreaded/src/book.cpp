#include <iostream>
#include <algorithm>
#include <cstdint>
#include <optional>

#include "messages.h"
#include "orders.h"
#include "aliases.h"
#include "book.h"

void OrderBook::reduceShares(Order& updatingOrder, Quantity sharesToReduce) {
    if (sharesToReduce > updatingOrder.shares) {  // invalid reduction
        std::cerr << "Data is invalid as the shares remaining is negative.\n";
        return;
    }
    else if (sharesToReduce == updatingOrder.shares) {  // no shares remain
        deleteOrder(updatingOrder.id);
        return;
    }
    else {
        updatingOrder.shares -= sharesToReduce;  // perform share reduction
    }
}

OrderBook::OrderBook(const StockStats& stockInfo, std::int32_t tickSize)
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

std::optional<Price> OrderBook::getBestBid() {  // highest price willing to buy at
    for (std::uint32_t i = bidLadder.size(); i > 0; i--) {
        if (bidLadder[i - 1].head != UINT32_MAX) {  // first populated order
            return chainPool[bidLadder[i - 1].head].price;
        }
    }
    return std::nullopt;
}

std::optional<Price> OrderBook::getBestAsk() {  // lowest price willing to sell at
    for (std::uint32_t i = 0; i < askLadder.size(); i++) {
        if (askLadder[i].head != UINT32_MAX) {  // first populated order
            return chainPool[askLadder[i].head].price;
        }
    }
    return std::nullopt;
}

void OrderBook::addOrder(OrderId id, Side side, Quantity shares, Price price) {
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

void OrderBook::cancelOrder(OrderId id, Quantity sharesToCancel) {
    auto it = orderMap.find(id);
    if (it == orderMap.end()) {  // order dne
        std::cerr << "CANCEL: Order does not exist in this book.\n";
        return;
    }
    reduceShares(chainPool[it->second], sharesToCancel);
}

void OrderBook::deleteOrder(OrderId id) {
    auto it = orderMap.find(id);
    if (it == orderMap.end()) {  // order dne
        std::cerr << "DELETE: Order does not exist in this book.\n";
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

void OrderBook::replaceOrder(OrderId initialId, OrderId newId, Quantity newShares, Price newPrice) {
    auto it = orderMap.find(initialId);
    if (it == orderMap.end()) {  // order dne
        std::cerr << "REPLACE: Order does not exist in this book.\n";
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

void OrderBook::executeOrder(OrderId id, Quantity sharesToExecute) {
    auto it = orderMap.find(id);
    if (it == orderMap.end()) {  // order dne
        std::cerr << "EXECUTE: Order does not exist in this book.\n";
        return;
    }
    reduceShares(chainPool[it->second], sharesToExecute);
}

void OrderBook::printAllOrders() {
    std::cout << "=========================\n";
    std::cout << "BID LADDER\n";
    for (std::uint32_t i = bidLadder.size(); i > 0; i--) {  // bids
        Price priceAtLevel = ((i - 1) * tickSize) + minPrice;  // get orig price
        PoolIndex current = bidLadder[i - 1].head;
        if (current == UINT32_MAX) {
            continue;
        }

        std::uint32_t totalOrders = 0;
        Quantity totalShares = 0;
        while (current != UINT32_MAX) {  // accumulate level info
            totalOrders++;
            totalShares += chainPool[current].shares;
            current = chainPool[current].nextIndex;
        }

        std::cout << priceAtLevel << ": " << totalOrders << " order(s), " << totalShares << " share(s)\n";
    }

    std::cout << "\nASK LADDER\n";
    for (std::uint32_t i = 0; i < askLadder.size(); i++) {  // asks
        Price priceAtLevel = (i * tickSize) + minPrice;  // get orig price
        PoolIndex current = askLadder[i].head;
        if (current == UINT32_MAX) {
            continue;
        }

        std::uint32_t totalOrders = 0;
        Quantity totalShares = 0;
        while (current != UINT32_MAX) {  // accumulate level info
            totalOrders++;
            totalShares += chainPool[current].shares;
            current = chainPool[current].nextIndex;
        }

        std::cout << priceAtLevel << ": " << totalOrders << " order(s), " << totalShares << " share(s)\n";
    }
    std::cout << "=========================\n";
}