#ifndef ORDER_STRUCTS
#define ORDER_STRUCTS

#include <cstdint>

using OrderId = std::uint64_t;
using Quantity = std::int32_t;
using Price = std::int32_t;
using PoolIndex = std::uint32_t;
using Side = char;

struct Order {
    OrderId id;
    Side side;
    Quantity shares;
    Price price;
    PoolIndex nextIndex = UINT32_MAX;
    PoolIndex prevIndex = UINT32_MAX;
};

struct StockStats {
    Price minPrice;
    Price maxPrice;
    std::uint32_t orderCount;
};

struct PriceLevelInfo {
    PoolIndex head = UINT32_MAX;
    PoolIndex tail = UINT32_MAX;
};

#endif