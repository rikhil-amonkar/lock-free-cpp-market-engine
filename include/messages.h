#ifndef MESSAGE_TYPES
#define MESSAGE_TYPES

#include <string>
#include <cstdint>

#include "../include/aliases.h"

struct BaseMessage {
    std::string type;
    std::uint64_t index;
};

struct SystemEvent {
    StockIndex stockIndex;
    TrackNumber trackNumber;
    Timestamp timestamp;
    EventCode eventCode;

    SystemEvent(StockIndex s, TrackNumber t, Timestamp time, EventCode e)
        : stockIndex(s)
        , trackNumber(t)
        , timestamp(time)
        , eventCode(e) {
    }
};

struct AddOrder {
    StockIndex stockIndex;
    TrackNumber trackNumber;
    Timestamp timestamp;
    OrderId orderId;
    Side side;
    Quantity shares;
    Ticker ticker;
    Price price;

    AddOrder(StockIndex s, TrackNumber t, Timestamp time, OrderId id, Side side, Quantity q, Ticker tick, Price p)
        : stockIndex(s)
        , trackNumber(t)
        , timestamp(time)
        , orderId(id)
        , side(side)
        , shares(q)
        , ticker(tick)
        , price(p) {
    }
};

struct AddOrderMPID {
    StockIndex stockIndex;
    TrackNumber trackNumber;
    Timestamp timestamp;
    OrderId orderId;
    Side side;
    Quantity shares;
    Ticker ticker;
    Price price;
    MPID mpid;

    AddOrderMPID(StockIndex s, TrackNumber t, Timestamp time, OrderId id, Side side, Quantity q, Ticker tick, Price p, MPID m)
        : stockIndex(s)
        , trackNumber(t)
        , timestamp(time)
        , orderId(id)
        , side(side)
        , shares(q)
        , ticker(tick)
        , price(p)
        , mpid(m) {
    }
};

struct OrderExecuted {
    StockIndex stockIndex;
    TrackNumber trackNumber;
    Timestamp timestamp;
    OrderId orderId;
    Quantity sharesTraded;
    MatchId matchId;

    OrderExecuted(StockIndex s, TrackNumber t, Timestamp time, OrderId id, Quantity q, MatchId m)
        : stockIndex(s)
        , trackNumber(t)
        , timestamp(time)
        , orderId(id)
        , sharesTraded(q)
        , matchId(m) {
    }
};

struct OrderExecutedPrice {
    StockIndex stockIndex;
    TrackNumber trackNumber;
    Timestamp timestamp;
    OrderId orderId;
    Quantity sharesTraded;
    MatchId matchId;
    Printable printable;
    Price priceTradedAt;

    OrderExecutedPrice(StockIndex s, TrackNumber t, Timestamp time, OrderId id, Quantity q, MatchId m, Printable print, Price p)
        : stockIndex(s)
        , trackNumber(t)
        , timestamp(time)
        , orderId(id)
        , sharesTraded(q)
        , matchId(m)
        , printable(print)
        , priceTradedAt(p) {
    }
};

struct OrderCancel {
    StockIndex stockIndex;
    TrackNumber trackNumber;
    Timestamp timestamp;
    OrderId orderId;
    Quantity canceledShares;

    OrderCancel(StockIndex s, TrackNumber t, Timestamp time, OrderId id, Quantity q)
        : stockIndex(s)
        , trackNumber(t)
        , timestamp(time)
        , orderId(id)
        , canceledShares(q) {
    }
};

struct OrderDelete {
    StockIndex stockIndex;
    TrackNumber trackNumber;
    Timestamp timestamp;
    OrderId orderId;

    OrderDelete(StockIndex s, TrackNumber t, Timestamp time, OrderId id)
        : stockIndex(s)
        , trackNumber(t)
        , timestamp(time)
        , orderId(id) {
    }
};

struct OrderReplace {
    StockIndex stockIndex;
    TrackNumber trackNumber;
    Timestamp timestamp;
    OrderId initOrderId;
    OrderId newOrderId;
    Quantity newShares;
    Price newPrice;

    OrderReplace(StockIndex s, TrackNumber t, Timestamp time, OrderId oid, OrderId nid, Quantity q, Price p)
        : stockIndex(s)
        , trackNumber(t)
        , timestamp(time)
        , initOrderId(oid)
        , newOrderId(nid)
        , newShares(q)
        , newPrice(p) {
    }
};

#endif