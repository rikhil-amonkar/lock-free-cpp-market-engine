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
#include <variant>

#include "readers.h"
#include "messages.h"
#include "orders.h"
#include "aliases.h"

class DataParser {
    void processDataStream(const std::string filename);

public:
    const std::string filename;

    // store all messages (to maintain order across types)
    std::vector < std::variant<AddOrder, ExecuteOrder, CancelOrder, DeleteOrder, ReplaceOrder>>& messages;

    // map stocks to running price bounds and order counts
    std::unordered_map<StockIndex, StockStats>& stockInfo;

    DataParser(
        const std::string f,
        std::vector < std::variant<AddOrder, ExecuteOrder, CancelOrder, DeleteOrder, ReplaceOrder>>& m,
        std::unordered_map<StockIndex, StockStats>& s
    );

    void getMessageAndStockData();
};

#endif