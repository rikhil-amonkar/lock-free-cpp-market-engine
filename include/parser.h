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

#include "../include/readers.h"
#include "../include/messages.h"
#include "../include/orders.h"

using StockIndex = std::uint16_t;
using TrackNumber = std::uint16_t;
using Timestamp = std::uint64_t;
using OrderId = std::uint64_t;
using MatchId = std::uint64_t;
using Quantity = std::int32_t;
using Price = std::int32_t;
using ticker = std::string;
using MPID = std::string;
using Side = char;
using EventCode = char;
using Printable = char;

class DataParser {

    // storage vectors for messages
    std::vector<SystemEvent> systemEvents;
    std::vector<AddOrder> addOrders;
    std::vector<AddOrderMPID> addOrdersMPID;
    std::vector<OrderExecuted> executeOrders;
    std::vector<OrderExecutedPrice> executeOrdersPrice;
    std::vector<OrderCancel> cancelOrders;
    std::vector<OrderDelete> deleteOrders;
    std::vector<OrderReplace> replaceOrders;

    // store all messages (to maintain order across types)
    std::vector<BaseMessage> allMessages;

    // map stocks to running price bounds and order counts
    std::unordered_map<StockIndex, StockStats> stockInfo;

private:
    void processDataStream(const std::string filename) {

        // for bin file + move ptr to end
        std::ifstream file(filename, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            std::cerr << "Failed to open file.\n";
            return;
        }

        std::streamsize fileSize = file.tellg();  // get size of file
        file.seekg(0, std::ios::beg);  // move ptr to beg of file

        std::vector<char> buffer(fileSize);  // allocate buffer
        if (file.read(buffer.data(), fileSize)) {
            std::cout << "Successfully read " << fileSize << " bytes of data into memory.\n";
        }

        std::int32_t remainingBytes = buffer.size();
        std::uint32_t offset = 0;

        while (remainingBytes > 1) {
            std::uint16_t lengthPrefix = read_16_bit_int_be(buffer, offset);
            if (offset + lengthPrefix > fileSize) {  // invalid indexing check
                std::cerr << "Byte index is out of range.\n";
                break;
            }

            std::int32_t messageStart = offset;  // mark orig pos
            char messageType = read_8_bit_int_be(buffer, offset);

            // system event
            if (messageType == 'S') {
                StockIndex stockIndex = read_16_bit_int_be(buffer, offset);
                TrackNumber trackNumber = read_16_bit_int_be(buffer, offset);
                Timestamp timestamp = read_48_bit_int_be(buffer, offset);
                EventCode eventCode = read_8_bit_int_be(buffer, offset);

                SystemEvent systemEventMessage(stockIndex, trackNumber, timestamp, eventCode);
                systemEvents.push_back(systemEventMessage);

                BaseMessage baseMessage{ "SystemEvent", systemEvents.size() - 1 };
                allMessages.push_back(baseMessage);
            }
            // add order
            else if (messageType == 'A') {
                StockIndex stockIndex = read_16_bit_int_be(buffer, offset);
                TrackNumber trackNumber = read_16_bit_int_be(buffer, offset);
                Timestamp timestamp = read_48_bit_int_be(buffer, offset);
                OrderId orderId = read_64_bit_int_be(buffer, offset);
                Side side = read_8_bit_int_be(buffer, offset);
                Quantity shares = read_32_bit_int_be(buffer, offset);
                Ticker ticker = read_ascii_field(buffer, offset, 8);
                Price price = read_32_bit_int_be(buffer, offset);

                AddOrder addOrderMessage(stockIndex, trackNumber, timestamp, orderId, side, shares, ticker, price);
                addOrders.push_back(addOrderMessage);

                allMessages.push_back(BaseMessage{ "AddOrder", addOrders.size() - 1 });

                // add or update stock specific info for map
                auto it = stockInfo.find(stockIndex);
                if (it == stockInfo.end()) {
                    stockInfo.insert({ stockIndex, StockStats{ price, price, 1 } });
                }
                else {  // min-max update and increment
                    it->second.minPrice = std::min(price, it->second.minPrice);
                    it->second.maxPrice = std::min(price, it->second.maxPrice);
                    it->second.orderCount += 1;
                }
            }
            // add order with mpid
            else if (messageType == 'F') {
                StockIndex stockIndex = read_16_bit_int_be(buffer, offset);
                TrackNumber trackNumber = read_16_bit_int_be(buffer, offset);
                Timestamp timestamp = read_48_bit_int_be(buffer, offset);
                OrderId orderId = read_64_bit_int_be(buffer, offset);
                Side side = read_8_bit_int_be(buffer, offset);
                Quantity shares = read_32_bit_int_be(buffer, offset);
                Ticker ticker = read_ascii_field(buffer, offset, 8);
                Price price = read_32_bit_int_be(buffer, offset);
                MPID mpid = read_ascii_field(buffer, offset, 4);

                AddOrderMPID addOrderMPIDMessage(stockIndex, trackNumber, timestamp, orderId, side, shares, ticker, price, mpid);
                addOrdersMPID.push_back(addOrderMPIDMessage);

                allMessages.push_back(BaseMessage{ "AddOrderMPID", addOrdersMPID.size() - 1 });

                // add or update stock specific info for map
                auto it = stockInfo.find(stockIndex);
                if (it == stockInfo.end()) {
                    stockInfo.insert({ stockIndex, StockStats{ price, price, 1 } });
                }
                else {  // min-max update and increment
                    it->second.minPrice = std::min(price, it->second.minPrice);
                    it->second.maxPrice = std::min(price, it->second.maxPrice);
                    it->second.orderCount += 1;
                }
            }
            // execute order
            else if (messageType == 'E') {
                StockIndex stockIndex = read_16_bit_int_be(buffer, offset);
                TrackNumber trackNumber = read_16_bit_int_be(buffer, offset);
                Timestamp timestamp = read_48_bit_int_be(buffer, offset);
                OrderId orderId = read_64_bit_int_be(buffer, offset);
                Quantity sharesTraded = read_32_bit_int_be(buffer, offset);
                MatchId matchId = read_64_bit_int_be(buffer, offset);

                OrderExecuted executedOrderMessage(stockIndex, trackNumber, timestamp, orderId, sharesTraded, matchId);
                executeOrders.push_back(executedOrderMessage);

                allMessages.push_back(BaseMessage{ "OrderExecuted", executeOrders.size() - 1 });
            }
            // execute order with price
            else if (messageType == 'C') {
                StockIndex stockIndex = read_16_bit_int_be(buffer, offset);
                TrackNumber trackNumber = read_16_bit_int_be(buffer, offset);
                Timestamp timestamp = read_48_bit_int_be(buffer, offset);
                OrderId orderId = read_64_bit_int_be(buffer, offset);
                Quantity sharesTraded = read_32_bit_int_be(buffer, offset);
                MatchId matchId = read_64_bit_int_be(buffer, offset);
                Printable printable = read_8_bit_int_be(buffer, offset);
                Price priceTradedAt = read_32_bit_int_be(buffer, offset);

                OrderExecutedPrice executedOrderPriceMessage(stockIndex, trackNumber, timestamp, orderId, sharesTraded, matchId, printable, priceTradedAt);
                executeOrdersPrice.push_back(executedOrderPriceMessage);

                allMessages.push_back(BaseMessage{ "OrderExecutedPrice", executeOrdersPrice.size() - 1 });
            }
            // cancel order
            else if (messageType == 'X') {
                StockIndex stockIndex = read_16_bit_int_be(buffer, offset);
                TrackNumber trackNumber = read_16_bit_int_be(buffer, offset);
                Timestamp timestamp = read_48_bit_int_be(buffer, offset);
                OrderId orderId = read_64_bit_int_be(buffer, offset);
                Quantity canceledShares = read_32_bit_int_be(buffer, offset);

                OrderCancel cancelOrderMessage(stockIndex, trackNumber, timestamp, orderId, canceledShares);
                cancelOrders.push_back(cancelOrderMessage);

                allMessages.push_back(BaseMessage{ "OrderCancel", cancelOrders.size() - 1 });
            }
            // delete order
            else if (messageType == 'D') {
                StockIndex stockIndex = read_16_bit_int_be(buffer, offset);
                TrackNumber trackNumber = read_16_bit_int_be(buffer, offset);
                Timestamp timestamp = read_48_bit_int_be(buffer, offset);
                OrderId orderId = read_64_bit_int_be(buffer, offset);

                OrderDelete deleteOrderMessage(stockIndex, trackNumber, timestamp, orderId);
                deleteOrders.push_back(deleteOrderMessage);

                allMessages.push_back(BaseMessage{ "OrderDelete", deleteOrders.size() - 1 });
            }
            // replace order
            else if (messageType == 'U') {
                StockIndex stockIndex = read_16_bit_int_be(buffer, offset);
                TrackNumber trackNumber = read_16_bit_int_be(buffer, offset);
                Timestamp timestamp = read_48_bit_int_be(buffer, offset);
                OrderId initOrderId = read_64_bit_int_be(buffer, offset);
                OrderId newOrderId = read_64_bit_int_be(buffer, offset);
                Quantity newShares = read_32_bit_int_be(buffer, offset);
                Price newPrice = read_32_bit_int_be(buffer, offset);

                OrderReplace replaceOrderMessage(stockIndex, trackNumber, timestamp, initOrderId, newOrderId, newShares, newPrice);
                replaceOrders.push_back(replaceOrderMessage);

                allMessages.push_back(BaseMessage{ "OrderReplace", replaceOrders.size() - 1 });
            }

            // update read pos
            offset = messageStart + lengthPrefix;
            remainingBytes = fileSize - offset;
        }

    }

public:
    const std::string filename;

    DataParser(const std::string f) : filename(f) {};

    std::pair<std::vector<BaseMessage>, std::unordered_map<StockIndex, StockStats>> getMessageAndStockData() {
        processDataStream(filename);
        return { allMessages, stockInfo };
    }

};

#endif