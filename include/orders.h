#ifndef ORDER_STRUCTS
#define ORDER_STRUCTS

#include <cstdint>

#include "aliases.h"

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