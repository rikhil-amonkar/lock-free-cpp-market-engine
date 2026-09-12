#ifndef MESSAGE_TYPES
#define MESSAGE_TYPES

#include <string>
#include <cstdint>

using u64 = std::uint64_t;
using u32 = std::uint32_t;
using u16 = std::uint16_t;
using i32 = std::int32_t;
using i8 = std::int8_t;
using string = std::string;

struct SystemEvent {
    u16 stockLocate;
    u16 trackNumber;
    u64 timestamp;
    char eventCode;

    SystemEvent(u16 s, u16 t, u64 ns, char e)
        : stockLocate(s)
        , trackNumber(t)
        , timestamp(ns)
        , eventCode(e) {
    }
};

struct AddOrder {
    u16 stockLocate;
    u16 trackNumber;
    u64 timestamp;
    u64 orderRefNumber;
    char side;
    i32 shares;
    i32 price;

    AddOrder(u16 s, u16 t, u64 ns, u64 r, char bs, i32 q, i32 p)
        : stockLocate(s)
        , trackNumber(t)
        , timestamp(ns)
        , orderRefNumber(r)
        , side(bs)
        , shares(q)
        , price(p) {
    }
};

struct AddOrderMPID {
    u16 stockLocate;
    u16 trackNumber;
    u64 timestamp;
    u64 orderRefNumber;
    char side;
    i32 shares;
    i32 price;
    string mpid;

    AddOrderMPID(u16 s, u16 t, u64 ns, u64 r, char bs, i32 q, i32 p, string m)
        : stockLocate(s)
        , trackNumber(t)
        , timestamp(ns)
        , orderRefNumber(r)
        , side(bs)
        , shares(q)
        , price(p)
        , mpid(m) {
    }
};

struct OrderExecuted {
    u16 stockLocate;
    u16 trackNumber;
    u64 timestamp;
    u64 orderRefNumber;
    i32 sharesTraded;
    u64 matchId;

    OrderExecuted(u16 s, u16 t, u64 ns, u64 r, i32 q, u64 id)
        : stockLocate(s)
        , trackNumber(t)
        , timestamp(ns)
        , orderRefNumber(r)
        , sharesTraded(q)
        , matchId(id) {
    }
};

struct OrderExecutedPrice {
    u16 stockLocate;
    u16 trackNumber;
    u64 timestamp;
    u64 orderRefNumber;
    i32 sharesTraded;
    u64 matchId;
    char printable;
    i32 priceTradedAt;

    OrderExecutedPrice(u16 s, u16 t, u64 ns, u64 r, i32 q, u64 id, char d, i32 p)
        : stockLocate(s)
        , trackNumber(t)
        , timestamp(ns)
        , orderRefNumber(r)
        , sharesTraded(q)
        , matchId(id)
        , printable(d)
        , priceTradedAt(p) {
    }
};

struct OrderCancel {
    u16 stockLocate;
    u16 trackNumber;
    u64 timestamp;
    u64 orderRefNumber;
    i32 canceledShares;

    OrderCancel(u16 s, u16 t, u64 ns, u64 r, i32 q)
        : stockLocate(s)
        , trackNumber(t)
        , timestamp(ns)
        , orderRefNumber(r)
        , canceledShares(q) {
    }
};

struct OrderDelete {
    u16 stockLocate;
    u16 trackNumber;
    u64 timestamp;
    u64 orderRefNumber;

    OrderDelete(u16 s, u16 t, u64 ns, u64 r)
        : stockLocate(s)
        , trackNumber(t)
        , timestamp(ns)
        , orderRefNumber(r) {
    }
};

struct OrderReplace {
    u16 stockLocate;
    u16 trackNumber;
    u64 timestamp;
    u64 initOrderRefNumber;
    u64 newOrderRefNumber;
    i32 newShares;
    i32 newPrice;

    OrderReplace(u16 s, u16 t, u64 ns, u64 ir, u64 nr, i32 q, i32 p)
        : stockLocate(s)
        , trackNumber(t)
        , timestamp(ns)
        , initOrderRefNumber(ir)
        , newOrderRefNumber(nr)
        , newShares(q)
        , newPrice(p) {
    }
};

#endif