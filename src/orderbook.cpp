#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include <algorithm>
#include <cstdint>

#include "../include/readers.h"
#include "../include/messages.h"
#include "../include/orders.h"

using Price = std::int32_t;
using PoolIndex = std::uint32_t;

class OrderBook {
    std::vector<Order> chainPool;
    std::vector<PoolIndex> bidLadder;
    std::vector<PoolIndex> askLadder;
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
        bidLadder.resize(numLevels, UINT32_MAX);
        askLadder.resize(numLevels, UINT32_MAX);
        chainPool.reserve(stockInfo.orderCount);
    }
};

int main() {
    return 0;
}