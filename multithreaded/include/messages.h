#ifndef MESSAGE_TYPES
#define MESSAGE_TYPES

#include <string>
#include <variant>
#include <cstdint>

#include "aliases.h"

struct AddOrder {
    StockIndex index;
    OrderId id;
    Side side;
    Quantity shares;
    Price price;

    AddOrder(StockIndex i, OrderId d, Side s, Quantity q, Price p)
        : index(i)
        , id(d)
        , side(s)
        , shares(q)
        , price(p) {
    }
};

struct ExecuteOrder {
    StockIndex index;
    OrderId id;
    Quantity sharesTraded;

    ExecuteOrder(StockIndex i, OrderId d, Quantity q)
        : index(i)
        , id(d)
        , sharesTraded(q) {
    }
};

struct CancelOrder {
    StockIndex index;
    OrderId id;
    Quantity canceledShares;

    CancelOrder(StockIndex i, OrderId d, Quantity q)
        : index(i)
        , id(d)
        , canceledShares(q) {
    }
};

struct DeleteOrder {
    StockIndex index;
    OrderId id;

    DeleteOrder(StockIndex i, OrderId d)
        : index(i)
        , id(d) {
    }
};

struct ReplaceOrder {
    StockIndex index;
    OrderId oid;
    OrderId nid;
    Quantity newShares;
    Price newPrice;

    ReplaceOrder(StockIndex i, OrderId o, OrderId n, Quantity q, Price p)
        : index(i)
        , oid(o)
        , nid(n)
        , newShares(q)
        , newPrice(p) {
    }
};

using OrderTypes = std::variant<AddOrder, ExecuteOrder, CancelOrder, DeleteOrder, ReplaceOrder>;

#endif