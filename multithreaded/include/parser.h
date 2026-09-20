#ifndef ITCH_PARSER_CLASS
#define ITCH_PARSER_CLASS

#include <iostream>
#include <fstream>
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
#include "queue.h"

class DataParser {
    const std::vector<char>& buffer;

    // store all messages (to maintain order across types)
    ThreadSafeSharedQueue& queue;

public:
    DataParser(
        const std::vector<char>& b,
        ThreadSafeSharedQueue& q
    );

    void processDataStream();
};

#endif