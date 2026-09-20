#ifndef BUFFER_CLASS
#define BUFFER_CLASS

#include <vector>
#include <string>

class ByteBuffer {
    const std::string filename;

    std::unordered_map<StockIndex, StockStats>& stockInfo;

public:
    ByteBuffer(const std::string f, std::unordered_map<StockIndex, StockStats>& s);

    std::vector<char> createBuffer();
};

#endif