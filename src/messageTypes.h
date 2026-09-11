#ifndef MESSAGE_TYPES
#define MESSAGE_TYPES

struct SystemEvent {
    int stockLocate;
    int trackNumber;
    long long timestamp;
    char eventCode;

    SystemEvent(int s, int t, long long ns, char e)
        : stockLocate(s)
        , trackNumber(t)
        , timestamp(ns)
        , eventCode(e) {
    }
};
struct AddOrder {
    int stockLocate;
    int trackNumber;
    long long timestamp;
    long long orderRefNumber;
    char side;
    int shares;
    int price;

    AddOrder(int s, int t, long long ns, long long r, char bs, int q, int p)
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
    int stockLocate;
    int trackNumber;
    long long timestamp;
    long long orderRefNumber;
    char side;
    int shares;
    int price;
    std::string mpid;

    AddOrderMPID(int s, int t, long long ns, long long r, char bs, int q, int p, std::string m)
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
    int stockLocate;
    int trackNumber;
    long long timestamp;
    long long orderRefNumber;
    int sharesTraded;
    long long matchId;

    OrderExecuted(int s, int t, long long ns, long long r, int q, long long id)
        : stockLocate(s)
        , trackNumber(t)
        , timestamp(ns)
        , orderRefNumber(r)
        , sharesTraded(q)
        , matchId(id) {
    }
};
struct OrderExecutedPrice {
    int stockLocate;
    int trackNumber;
    long long timestamp;
    long long orderRefNumber;
    int sharesTraded;
    long long matchId;
    char printable;
    int priceTradedAt;

    OrderExecutedPrice(int s, int t, long long ns, long long r, int q, long long id, char d, int p)
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
    int stockLocate;
    int trackNumber;
    long long timestamp;
    long long orderRefNumber;
    int canceledShares;

    OrderCancel(int s, int t, long long ns, long long r, int q)
        : stockLocate(s)
        , trackNumber(t)
        , timestamp(ns)
        , orderRefNumber(r)
        , canceledShares(q) {
    }
};
struct OrderDelete {
    int stockLocate;
    int trackNumber;
    long long timestamp;
    long long orderRefNumber;

    OrderDelete(int s, int t, long long ns, long long r)
        : stockLocate(s)
        , trackNumber(t)
        , timestamp(ns)
        , orderRefNumber(r) {
    }
};
struct OrderReplace {
    int stockLocate;
    int trackNumber;
    long long timestamp;
    long long initOrderRefNumber;
    long long newOrderRefNumber;
    int newShares;
    int newPrice;

    OrderReplace(int s, int t, long long ns, long long ir, long long nr, int q, int p)
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