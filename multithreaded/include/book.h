#ifndef ORDER_BOOK_CLASS
#define ORDER_BOOK_CLASS

#include <vector>
#include <unordered_map>
#include <cstdint>
#include <optional>

#include "messages.h"
#include "orders.h"
#include "aliases.h"

class OrderBook {
    std::vector<Order> chainPool;
    std::vector<PriceLevelInfo> bidLadder;
    std::vector<PriceLevelInfo> askLadder;
    std::vector<PoolIndex> freeSlots;
    std::unordered_map<OrderId, PoolIndex> orderMap;

    Price minPrice, maxPrice;
    std::int32_t tickSize;
    std::uint32_t numLevels;

    void reduceShares(Order& updatingOrder, Quantity sharesToReduce);

public:
    OrderBook(const StockStats& stockInfo, std::int32_t tickSize = 1);

    std::optional<Price> getBestBid();

    std::optional<Price> getBestAsk();

    void addOrder(OrderId id, Side side, Quantity shares, Price price);

    void cancelOrder(OrderId id, Quantity sharesToCancel);

    void deleteOrder(OrderId id);

    void replaceOrder(OrderId initialId, OrderId newId, Quantity newShares, Price newPrice);

    void executeOrder(OrderId id, Quantity sharesToExecute);

    void printAllOrders();
};

#endif