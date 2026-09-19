#ifndef ITCH_PARSER_CLASS
#define ITCH_PARSER_CLASS

#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <string>
#include <algorithm>
#include <cstdint>
#include <utility>

#include "readers.h"
#include "messages.h"
#include "orders.h"
#include "aliases.h"

class DataParser {
    void processDataStream(const std::string filename);

public:
    const std::string filename;

    // storage vectors for messages
    std::vector<SystemEvent>& systemEvents;
    std::vector<AddOrder>& addOrders;
    std::vector<AddOrderMPID>& addOrdersMPID;
    std::vector<OrderExecuted>& executeOrders;
    std::vector<OrderExecutedPrice>& executeOrdersPrice;
    std::vector<OrderCancel>& cancelOrders;
    std::vector<OrderDelete>& deleteOrders;
    std::vector<OrderReplace>& replaceOrders;

    // store all messages (to maintain order across types)
    std::vector<BaseMessage>& allMessages;

    // map stocks to running price bounds and order counts
    std::unordered_map<StockIndex, StockStats>& stockInfo;

    DataParser(
        const std::string f,
        std::vector<SystemEvent>& systemEvents,
        std::vector<AddOrder>& addOrders,
        std::vector<AddOrderMPID>& addOrdersMPID,
        std::vector<OrderExecuted>& executeOrders,
        std::vector<OrderExecutedPrice>& executeOrdersPrice,
        std::vector<OrderCancel>& cancelOrders,
        std::vector<OrderDelete>& deleteOrders,
        std::vector<OrderReplace>& replaceOrders,
        std::vector<BaseMessage>& allMessages,
        std::unordered_map<StockIndex, StockStats>& stockInfo
    );

    void getMessageAndStockData();
};

#endif