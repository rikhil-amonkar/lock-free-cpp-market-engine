#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <format>

int extract_length_prefix(std::vector<char>& buffer, int& offset) {
    std::string hex = {
        std::format(
            "{:02x}{:02x}\n",
            // prevent sign-extension with char to hex
            static_cast<unsigned char>(buffer[offset]),
            static_cast<unsigned char>(buffer[offset + 1])
        )
    };
    int size = std::stoi(hex, nullptr, 16);  // hex to dec
    offset += 2;

    return size;
}

int main() {

    const std::string FILENAME = "./sample.bin";
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
    int offset = 0;

    /* xxd 16-byte output format (NOT each message already split)
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

        std::string message;
        for (int i = offset; i < offset + lengthPrefix; i++) {
            message += std::format("{:02x} ", buffer[i]);  // build message with byte pairs
        }
        offset += lengthPrefix;
        remainingBytes = fileSize - offset;

        std::cout << lengthPrefix << " " << message << "\n";
    }

    return 0;
}