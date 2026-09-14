#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <format>
#include <algorithm>
#include <cstdint>

#include "../include/bit_readers.h"
#include "../include/message_types.h"

using StockLocate = std::uint16_t;
using TrackNumber = std::uint16_t;
using Timestamp = std::uint64_t;
using OrderId = std::uint64_t;
using MatchId = std::uint64_t;
using Quantity = std::int32_t;
using Price = std::int32_t;
using ticker = std::string;
using MPID = std::string;

int main() {

    const std::string FILENAME = "./sample.bin";

    // for bin file + move ptr to end
    std::ifstream file(FILENAME, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Failed to open file.\n";
        return 1;
    }

    std::streamsize fileSize = file.tellg();  // get size of file
    file.seekg(0, std::ios::beg);  // move ptr to beg of file

    std::vector<char> buffer(fileSize);  // allocate buffer
    if (file.read(buffer.data(), fileSize)) {
        std::cout << "Successfully read " << fileSize << " bytes of data into memory.\n";
    }

    std::int32_t remainingBytes = buffer.size();
    std::uint32_t offset = 0;

    // storage vectors for messages
    std::vector<SystemEvent> systemEvents;
    std::vector<AddOrder> addOrders;
    std::vector<AddOrderMPID> addOrdersMPID;
    std::vector<OrderExecuted> executeOrders;
    std::vector<OrderExecutedPrice> executeOrdersPrice;
    std::vector<OrderCancel> cancelOrders;
    std::vector<OrderDelete> deleteOrders;
    std::vector<OrderReplace> replaceOrders;

    while (remainingBytes > 1) {
        std::uint16_t lengthPrefix = read_16_bit_int_be(buffer, offset);
        if (offset + lengthPrefix > fileSize) {  // invalid indexing check
            std::cerr << "Byte index is out of range.\n";
            break;
        }

        std::int32_t messageStart = offset;
        char messageType = read_8_bit_int_be(buffer, offset);

        // system event
        if (messageType == 'S') {
            StockLocate stockLocate = read_16_bit_int_be(buffer, offset);
            TrackNumber trackNumber = read_16_bit_int_be(buffer, offset);
            Timestamp timestamp = read_48_bit_int_be(buffer, offset);
            char eventCode = read_8_bit_int_be(buffer, offset);

            SystemEvent systemEventMessage(stockLocate, trackNumber, timestamp, eventCode);
            systemEvents.push_back(systemEventMessage);
        }
        // add order
        else if (messageType == 'A') {
            StockLocate stockLocate = read_16_bit_int_be(buffer, offset);
            TrackNumber trackNumber = read_16_bit_int_be(buffer, offset);
            Timestamp timestamp = read_48_bit_int_be(buffer, offset);
            OrderId orderId = read_64_bit_int_be(buffer, offset);
            char side = read_8_bit_int_be(buffer, offset);
            Quantity shares = read_32_bit_int_be(buffer, offset);
            Ticker ticker = read_ascii_field(buffer, offset, 8);
            Price price = read_32_bit_int_be(buffer, offset);

            AddOrder addOrderMessage(stockLocate, trackNumber, timestamp, orderId, side, shares, ticker, price);
            addOrders.push_back(addOrderMessage);
        }
        // add order with mpid
        else if (messageType == 'F') {
            StockLocate stockLocate = read_16_bit_int_be(buffer, offset);
            TrackNumber trackNumber = read_16_bit_int_be(buffer, offset);
            Timestamp timestamp = read_48_bit_int_be(buffer, offset);
            OrderId orderId = read_64_bit_int_be(buffer, offset);
            char side = read_8_bit_int_be(buffer, offset);
            Quantity shares = read_32_bit_int_be(buffer, offset);
            Ticker ticker = read_ascii_field(buffer, offset, 8);
            Price price = read_32_bit_int_be(buffer, offset);
            MPID mpid = read_ascii_field(buffer, offset, 4);

            AddOrderMPID addOrderMPIDMessage(stockLocate, trackNumber, timestamp, orderId, side, shares, ticker, price, mpid);
            addOrdersMPID.push_back(addOrderMPIDMessage);
        }
        // execute order
        else if (messageType == 'E') {
            StockLocate stockLocate = read_16_bit_int_be(buffer, offset);
            TrackNumber trackNumber = read_16_bit_int_be(buffer, offset);
            Timestamp timestamp = read_48_bit_int_be(buffer, offset);
            OrderId orderId = read_64_bit_int_be(buffer, offset);
            Quantity sharesTraded = read_32_bit_int_be(buffer, offset);
            MatchId matchId = read_64_bit_int_be(buffer, offset);

            OrderExecuted executedOrderMessage(stockLocate, trackNumber, timestamp, orderId, sharesTraded, matchId);
            executeOrders.push_back(executedOrderMessage);
        }
        // execute order with price
        else if (messageType == 'C') {
            StockLocate stockLocate = read_16_bit_int_be(buffer, offset);
            TrackNumber trackNumber = read_16_bit_int_be(buffer, offset);
            Timestamp timestamp = read_48_bit_int_be(buffer, offset);
            OrderId orderId = read_64_bit_int_be(buffer, offset);
            Quantity sharesTraded = read_32_bit_int_be(buffer, offset);
            MatchId matchId = read_64_bit_int_be(buffer, offset);
            char printable = read_8_bit_int_be(buffer, offset);
            Price priceTradedAt = read_32_bit_int_be(buffer, offset);

            OrderExecutedPrice executedOrderPriceMessage(stockLocate, trackNumber, timestamp, orderId, sharesTraded, matchId, printable, priceTradedAt);
            executeOrdersPrice.push_back(executedOrderPriceMessage);
        }
        // cancel order
        else if (messageType == 'X') {
            StockLocate stockLocate = read_16_bit_int_be(buffer, offset);
            TrackNumber trackNumber = read_16_bit_int_be(buffer, offset);
            Timestamp timestamp = read_48_bit_int_be(buffer, offset);
            OrderId orderId = read_64_bit_int_be(buffer, offset);
            Quantity canceledShares = read_32_bit_int_be(buffer, offset);

            OrderCancel cancelOrderMessage(stockLocate, trackNumber, timestamp, orderId, canceledShares);
            cancelOrders.push_back(cancelOrderMessage);
        }
        // delete order
        else if (messageType == 'D') {
            StockLocate stockLocate = read_16_bit_int_be(buffer, offset);
            TrackNumber trackNumber = read_16_bit_int_be(buffer, offset);
            Timestamp timestamp = read_48_bit_int_be(buffer, offset);
            OrderId orderId = read_64_bit_int_be(buffer, offset);

            OrderDelete deleteOrderMessage(stockLocate, trackNumber, timestamp, orderId);
            deleteOrders.push_back(deleteOrderMessage);
        }
        // replace order
        else if (messageType == 'U') {
            StockLocate stockLocate = read_16_bit_int_be(buffer, offset);
            TrackNumber trackNumber = read_16_bit_int_be(buffer, offset);
            Timestamp timestamp = read_48_bit_int_be(buffer, offset);
            OrderId initOrderId = read_64_bit_int_be(buffer, offset);
            OrderId newOrderId = read_64_bit_int_be(buffer, offset);
            Quantity newShares = read_32_bit_int_be(buffer, offset);
            Price newPrice = read_32_bit_int_be(buffer, offset);

            OrderReplace replaceOrderMessage(stockLocate, trackNumber, timestamp, initOrderId, newOrderId, newShares, newPrice);
            replaceOrders.push_back(replaceOrderMessage);
        }

        offset = messageStart + lengthPrefix;
        remainingBytes = fileSize - offset;
    }

    return 0;
}