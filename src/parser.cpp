#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <format>
#include <algorithm>

int extract_length_prefix(std::vector<char>& buffer, size_t& offset) {
    std::string hex = {
        std::format(  // prevent sign-extension with char to hex
            "{:02x}{:02x}\n",
            static_cast<unsigned char>(buffer[offset]),
            static_cast<unsigned char>(buffer[offset + 1])
        )
    };
    int size = std::stoi(hex, nullptr, 16);  // hex to dec
    offset += 2;

    return size;
}

long long read_64_bit_int_be(std::vector<char>& buffer, size_t& offset) {
    long long value = 0;
    for (size_t i = 0; i < 8; i++) {
        value = (value << 8) | static_cast<unsigned char>(buffer[offset + i]);
    }
    offset += 8;

    return value;
}

long long read_48_bit_int_be(std::vector<char>& buffer, size_t& offset) {
    long long value = 0;
    for (size_t i = 0; i < 6; i++) {
        value = (value << 8) | static_cast<unsigned char>(buffer[offset + i]);
    }
    offset += 6;

    return value;
}

int read_32_bit_int_be(std::vector<char>& buffer, size_t& offset) {
    long long value = 0;
    for (size_t i = 0; i < 4; i++) {
        value = (value << 8) | static_cast<unsigned char>(buffer[offset + i]);
    }
    offset += 4;

    return value;
}

int read_16_bit_int_be(std::vector<char>& buffer, size_t& offset) {
    int value = 0;
    for (size_t i = 0; i < 2; i++) {
        value = (value << 8) | static_cast<unsigned char>(buffer[offset + i]);
    }
    offset += 2;

    return value;
}

int read_8_bit_int_be(std::vector<char>& buffer, size_t& offset) {
    int value = static_cast<unsigned char>(buffer[offset]);
    offset += 1;

    return value;
}

std::string read_ascii_field(std::vector<char>& buffer, size_t& offset, size_t length) {
    std::string value;
    for (size_t i = 0; i < length; i++) {
        value += buffer[offset + i];  // raw char
    }
    offset += length;
    std::erase_if(value, [](unsigned char c) { return std::isspace(c); });

    return value;
}

int main() {

    const std::string FILENAME = "./sample.bin";
    // const std::string FILENAME = "../data/20190730.BX_ITCH_50.bin";
    const int BYTES_IN_MB = 1000000;

    std::ifstream file(FILENAME, std::ios::binary | std::ios::ate);  // for bin file + move ptr to end
    if (!file.is_open()) {
        std::cerr << "Failed to open file.\n";
        return 1;
    }

    std::streamsize fileSize = file.tellg();  // get size of file
    file.seekg(0, std::ios::beg);  // move ptr to beg of file

    std::vector<char> buffer(fileSize);  // allocate buffer
    if (file.read(buffer.data(), fileSize)) {
        std::cout << "Successfully read " << (fileSize / BYTES_IN_MB) << " MB of data into memory.\n";
    }

    int remainingBytes = buffer.size();
    size_t offset = 0;

    /* xxd 16 byte output format (NOT each message already split)
    00000000: 000c 5300 0000 000a 2e74 d456 a04f 0027  ..S......t.V.O.'
    00000010: 5200 0100 000a 65af 1fc0 cf41 2020 2020  R.....e....A
    00000020: 2020 204e 2000 0000 644e 435a 2050 4e20     N ...dNCZ PN
    00000030: 314e 0000 0000 4e00 1948 0001 0000 0a65  1N....N..H.....e
    */

    while (remainingBytes > 1) {
        int lengthPrefix = extract_length_prefix(buffer, offset);
        if (offset + lengthPrefix > fileSize) {  // invalid indexing check
            std::cerr << "Byte index is out of range.\n";
            break;
        }

        size_t messageStart = offset;
        char messageType = read_8_bit_int_be(buffer, offset);

        // event handling conditional
        if (messageType == 'S') {  // system event
            int stockLocate = read_16_bit_int_be(buffer, offset);
            int trackNumber = read_16_bit_int_be(buffer, offset);
            long long timestamp = read_48_bit_int_be(buffer, offset);
            char eventCode = read_8_bit_int_be(buffer, offset);

            std::cout << std::format(
                "{} {} {} {} {}\n",
                messageType, stockLocate, trackNumber,
                timestamp, eventCode
            );
            if (eventCode == 'C') {
                std::cout << "End of day.\n";
            }
        }
        else if (messageType == 'A') {  // add order
            int stockLocate = read_16_bit_int_be(buffer, offset);
            int trackNumber = read_16_bit_int_be(buffer, offset);
            long long timestamp = read_48_bit_int_be(buffer, offset);
            long long orderRefNumber = read_64_bit_int_be(buffer, offset);
            char side = read_8_bit_int_be(buffer, offset);
            int shared = read_32_bit_int_be(buffer, offset);
            std::string stockName = read_ascii_field(buffer, offset, 8);
            int price = read_32_bit_int_be(buffer, offset);

            // std::cout << std::format(
            //     "{} {} {} {} {} {} {} {}\n",
            //     messageType, stockLocate, trackNumber,
            //     timestamp, orderRefNumber, side,
            //     shared, stockName, price
            // );
        }
        else if (messageType == 'F') {  // add order with mpid
            int stockLocate = read_16_bit_int_be(buffer, offset);
            int trackNumber = read_16_bit_int_be(buffer, offset);
            long long timestamp = read_48_bit_int_be(buffer, offset);
            long long orderRefNumber = read_64_bit_int_be(buffer, offset);
            char side = read_8_bit_int_be(buffer, offset);
            int shared = read_32_bit_int_be(buffer, offset);
            std::string stockName = read_ascii_field(buffer, offset, 8);
            int price = read_32_bit_int_be(buffer, offset);
            std::string mpid = read_ascii_field(buffer, offset, 4);

            // std::cout << std::format(
            //     "{} {} {} {} {} {} {} {} {}\n",
            //     messageType, stockLocate, trackNumber,
            //     timestamp, orderRefNumber, side,
            //     shared, stockName, price, mpid
            // );
        }
        else if (messageType == 'E') {  // execute order
            int stockLocate = read_16_bit_int_be(buffer, offset);
            int trackNumber = read_16_bit_int_be(buffer, offset);
            long long timestamp = read_48_bit_int_be(buffer, offset);
            long long orderRefNumber = read_64_bit_int_be(buffer, offset);
            int sharesTraded = read_32_bit_int_be(buffer, offset);
            long long matchId = read_64_bit_int_be(buffer, offset);

            // std::cout << std::format(
            //     "{} {} {} {} {} {} {}\n",
            //     messageType, stockLocate, trackNumber, timestamp,
            //     orderRefNumber, sharesTraded, matchId
            // );
        }
        else if (messageType == 'C') {  // execute order with price
            int stockLocate = read_16_bit_int_be(buffer, offset);
            int trackNumber = read_16_bit_int_be(buffer, offset);
            long long timestamp = read_48_bit_int_be(buffer, offset);
            long long orderRefNumber = read_64_bit_int_be(buffer, offset);
            int sharesTraded = read_32_bit_int_be(buffer, offset);
            long long matchId = read_64_bit_int_be(buffer, offset);
            char printable = read_8_bit_int_be(buffer, offset);
            int priceTradedAt = read_32_bit_int_be(buffer, offset);

            std::cout << std::format(
                "{} {} {} {} {} {} {} {} {}\n",
                messageType, stockLocate, trackNumber,
                timestamp, orderRefNumber, sharesTraded,
                matchId, printable, priceTradedAt
            );
        }

        offset = messageStart + lengthPrefix;
        remainingBytes = fileSize - offset;
    }

    return 0;
}