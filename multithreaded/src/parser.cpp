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
#include "parser.h"

void DataParser::processDataStream(const std::string filename) {

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
        std::uint16_t lengthPrefix = read_16_bit_int_be(buffer, offset);  // size of message
        if (offset + lengthPrefix > fileSize) {  // invalid indexing check
            std::cerr << "Byte index is out of range.\n";
            break;
        }

        std::int32_t messageStart = offset;  // mark orig pos
        char messageType = read_8_bit_int_be(buffer, offset);

        // add order
        if (messageType == 'A') {
            StockIndex index = read_16_bit_int_be(buffer, offset);
            TrackNumber trackNumber = read_16_bit_int_be(buffer, offset);
            Timestamp timestamp = read_48_bit_int_be(buffer, offset);
            OrderId id = read_64_bit_int_be(buffer, offset);
            Side side = read_8_bit_int_be(buffer, offset);
            Quantity shares = read_32_bit_int_be(buffer, offset);
            Ticker ticker = read_ascii_field(buffer, offset, 8);
            Price price = read_32_bit_int_be(buffer, offset);

            messages.push_back(AddOrder{ index, id, side, shares, price });

            // add or update stock specific info for map
            auto it = stockInfo.find(index);
            if (it == stockInfo.end()) {
                stockInfo.insert({ index, StockStats{ price, price, 1 } });
            }
            else {  // min-max update and increment
                it->second.minPrice = std::min(price, it->second.minPrice);
                it->second.maxPrice = std::max(price, it->second.maxPrice);
                it->second.orderCount++;
            }
        }
        // add order with mpid
        else if (messageType == 'F') {
            StockIndex index = read_16_bit_int_be(buffer, offset);
            TrackNumber trackNumber = read_16_bit_int_be(buffer, offset);
            Timestamp timestamp = read_48_bit_int_be(buffer, offset);
            OrderId id = read_64_bit_int_be(buffer, offset);
            Side side = read_8_bit_int_be(buffer, offset);
            Quantity shares = read_32_bit_int_be(buffer, offset);
            Ticker ticker = read_ascii_field(buffer, offset, 8);
            Price price = read_32_bit_int_be(buffer, offset);
            MPID mpid = read_ascii_field(buffer, offset, 4);

            messages.push_back(AddOrder{ index, id, side, shares, price });

            // add or update stock specific info for map
            auto it = stockInfo.find(index);
            if (it == stockInfo.end()) {
                stockInfo.insert({ index, StockStats{ price, price, 1 } });
            }
            else {  // min-max update and increment
                it->second.minPrice = std::min(price, it->second.minPrice);
                it->second.maxPrice = std::max(price, it->second.maxPrice);
                it->second.orderCount++;
            }
        }
        // execute order
        else if (messageType == 'E') {
            StockIndex index = read_16_bit_int_be(buffer, offset);
            TrackNumber trackNumber = read_16_bit_int_be(buffer, offset);
            Timestamp timestamp = read_48_bit_int_be(buffer, offset);
            OrderId id = read_64_bit_int_be(buffer, offset);
            Quantity sharesTraded = read_32_bit_int_be(buffer, offset);
            MatchId matchId = read_64_bit_int_be(buffer, offset);

            messages.push_back(ExecuteOrder{ index, id, sharesTraded });
        }
        // execute order with price
        else if (messageType == 'C') {
            StockIndex index = read_16_bit_int_be(buffer, offset);
            TrackNumber trackNumber = read_16_bit_int_be(buffer, offset);
            Timestamp timestamp = read_48_bit_int_be(buffer, offset);
            OrderId id = read_64_bit_int_be(buffer, offset);
            Quantity sharesTraded = read_32_bit_int_be(buffer, offset);
            MatchId matchId = read_64_bit_int_be(buffer, offset);
            Printable printable = read_8_bit_int_be(buffer, offset);
            Price priceTradedAt = read_32_bit_int_be(buffer, offset);

            messages.push_back(ExecuteOrder{ index, id, sharesTraded });
        }
        // cancel order
        else if (messageType == 'X') {
            StockIndex index = read_16_bit_int_be(buffer, offset);
            TrackNumber trackNumber = read_16_bit_int_be(buffer, offset);
            Timestamp timestamp = read_48_bit_int_be(buffer, offset);
            OrderId id = read_64_bit_int_be(buffer, offset);
            Quantity cancelledShares = read_32_bit_int_be(buffer, offset);

            messages.push_back(CancelOrder{ index, id, cancelledShares });
        }
        // delete order
        else if (messageType == 'D') {
            StockIndex index = read_16_bit_int_be(buffer, offset);
            TrackNumber trackNumber = read_16_bit_int_be(buffer, offset);
            Timestamp timestamp = read_48_bit_int_be(buffer, offset);
            OrderId id = read_64_bit_int_be(buffer, offset);

            messages.push_back(DeleteOrder{ index, id });
        }
        // replace order
        else if (messageType == 'U') {
            StockIndex index = read_16_bit_int_be(buffer, offset);
            TrackNumber trackNumber = read_16_bit_int_be(buffer, offset);
            Timestamp timestamp = read_48_bit_int_be(buffer, offset);
            OrderId oid = read_64_bit_int_be(buffer, offset);
            OrderId nid = read_64_bit_int_be(buffer, offset);
            Quantity newShares = read_32_bit_int_be(buffer, offset);
            Price newPrice = read_32_bit_int_be(buffer, offset);

            messages.push_back(ReplaceOrder{ index, oid, nid, newShares, newPrice });

            // update min/max (due to new price info)
            auto it = stockInfo.find(index);
            if (it != stockInfo.end()) {
                it->second.minPrice = std::min(it->second.minPrice, newPrice);
                it->second.maxPrice = std::max(it->second.maxPrice, newPrice);
            }
        }

        // update read pos
        offset = messageStart + lengthPrefix;
        remainingBytes = fileSize - offset;
    }
}

DataParser::DataParser(
    const std::string f,
    std::vector<std::variant<AddOrder, ExecuteOrder, CancelOrder, DeleteOrder, ReplaceOrder>>& m,
    std::unordered_map<StockIndex, StockStats>& s
)
    : filename(f)
    , messages(m)
    , stockInfo(s) {
};

void DataParser::getMessageAndStockData() {
    processDataStream(filename);
}