#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <format>
#include <algorithm>
#include <cstdint>

#include "../include/bit_readers.h"
#include "../include/message_types.h"

using u64 = std::uint64_t;
using u32 = std::uint32_t;
using u16 = std::uint16_t;
using i32 = std::int32_t;
using i8 = std::int8_t;
using string = std::string;

int main() {

    const string FILENAME = "./sample.bin";

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

    i32 remainingBytes = buffer.size();
    u32 offset = 0;

    // storage vectors for messages
    std::vector<SystemEvent> sysEvents;
    std::vector<AddOrder> addOrders;
    std::vector<AddOrderMPID> addOrdersMPID;
    std::vector<OrderExecuted> execOrders;
    std::vector<OrderExecutedPrice> priceExecOrders;
    std::vector<OrderCancel> cancelOrders;
    std::vector<OrderDelete> deleteOrders;
    std::vector<OrderReplace> replaceOrders;

    while (remainingBytes > 1) {
        u16 lengthPrefix = read_16_bit_int_be(buffer, offset);
        if (offset + lengthPrefix > fileSize) {  // invalid indexing check
            std::cerr << "Byte index is out of range.\n";
            break;
        }

        i32 messageStart = offset;
        char messageType = read_8_bit_int_be(buffer, offset);

        // system event
        if (messageType == 'S') {
            u16 stockLocate = read_16_bit_int_be(buffer, offset);
            u16 trackNumber = read_16_bit_int_be(buffer, offset);
            u64 timestamp = read_48_bit_int_be(buffer, offset);
            char eventCode = read_8_bit_int_be(buffer, offset);
        }
        // add order
        else if (messageType == 'A') {
            u16 stockLocate = read_16_bit_int_be(buffer, offset);
            u16 trackNumber = read_16_bit_int_be(buffer, offset);
            u64 timestamp = read_48_bit_int_be(buffer, offset);
            u64 orderRefNumber = read_64_bit_int_be(buffer, offset);
            char side = read_8_bit_int_be(buffer, offset);
            i32 shares = read_32_bit_int_be(buffer, offset);
            string stockName = read_ascii_field(buffer, offset, 8);
            i32 price = read_32_bit_int_be(buffer, offset);
        }
        // add order with mpid
        else if (messageType == 'F') {
            u16 stockLocate = read_16_bit_int_be(buffer, offset);
            u16 trackNumber = read_16_bit_int_be(buffer, offset);
            u64 timestamp = read_48_bit_int_be(buffer, offset);
            u64 orderRefNumber = read_64_bit_int_be(buffer, offset);
            char side = read_8_bit_int_be(buffer, offset);
            i32 shares = read_32_bit_int_be(buffer, offset);
            string stockName = read_ascii_field(buffer, offset, 8);
            i32 price = read_32_bit_int_be(buffer, offset);
            string mpid = read_ascii_field(buffer, offset, 4);
        }
        // execute order
        else if (messageType == 'E') {
            u16 stockLocate = read_16_bit_int_be(buffer, offset);
            u16 trackNumber = read_16_bit_int_be(buffer, offset);
            u64 timestamp = read_48_bit_int_be(buffer, offset);
            u64 orderRefNumber = read_64_bit_int_be(buffer, offset);
            i32 sharesTraded = read_32_bit_int_be(buffer, offset);
            u64 matchId = read_64_bit_int_be(buffer, offset);
        }
        // execute order with price
        else if (messageType == 'C') {
            u16 stockLocate = read_16_bit_int_be(buffer, offset);
            u16 trackNumber = read_16_bit_int_be(buffer, offset);
            u64 timestamp = read_48_bit_int_be(buffer, offset);
            u64 orderRefNumber = read_64_bit_int_be(buffer, offset);
            i32 sharesTraded = read_32_bit_int_be(buffer, offset);
            u64 matchId = read_64_bit_int_be(buffer, offset);
            char printable = read_8_bit_int_be(buffer, offset);
            i32 priceTradedAt = read_32_bit_int_be(buffer, offset);
        }
        // cancel order
        else if (messageType == 'X') {
            u16 stockLocate = read_16_bit_int_be(buffer, offset);
            u16 trackNumber = read_16_bit_int_be(buffer, offset);
            u64 timestamp = read_48_bit_int_be(buffer, offset);
            u64 orderRefNumber = read_64_bit_int_be(buffer, offset);
            i32 canceledShares = read_32_bit_int_be(buffer, offset);
        }
        // delete order
        else if (messageType == 'D') {
            u16 stockLocate = read_16_bit_int_be(buffer, offset);
            u16 trackNumber = read_16_bit_int_be(buffer, offset);
            u64 timestamp = read_48_bit_int_be(buffer, offset);
            u64 orderRefNumber = read_64_bit_int_be(buffer, offset);
        }
        // replace order
        else if (messageType == 'U') {
            u16 stockLocate = read_16_bit_int_be(buffer, offset);
            u16 trackNumber = read_16_bit_int_be(buffer, offset);
            u64 timestamp = read_48_bit_int_be(buffer, offset);
            u64 initOrderRefNumber = read_64_bit_int_be(buffer, offset);
            u64 newOrderRefNumber = read_64_bit_int_be(buffer, offset);
            i32 newShares = read_32_bit_int_be(buffer, offset);
            i32 newPrice = read_32_bit_int_be(buffer, offset);
        }

        offset = messageStart + lengthPrefix;
        remainingBytes = fileSize - offset;
    }

    return 0;
}