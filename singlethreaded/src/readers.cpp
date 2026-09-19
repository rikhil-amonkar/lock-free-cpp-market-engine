#include <string>
#include <vector>
#include <cstdint>

#include "readers.h"

std::uint64_t read_64_bit_int_be(std::vector<char>& buffer, std::uint32_t& offset) {
    std::uint64_t value = 0;
    for (std::uint32_t i = 0; i < 8; i++) {
        value = (value << 8) | static_cast<unsigned char>(buffer[offset + i]);
    }
    offset += 8;

    return value;
}

std::uint64_t read_48_bit_int_be(std::vector<char>& buffer, std::uint32_t& offset) {
    std::uint64_t value = 0;
    for (std::uint32_t i = 0; i < 6; i++) {
        value = (value << 8) | static_cast<unsigned char>(buffer[offset + i]);
    }
    offset += 6;

    return value;
}

std::int32_t read_32_bit_int_be(std::vector<char>& buffer, std::uint32_t& offset) {
    std::int32_t value = 0;
    for (std::uint32_t i = 0; i < 4; i++) {
        value = (value << 8) | static_cast<unsigned char>(buffer[offset + i]);
    }
    offset += 4;

    return value;
}

std::uint16_t read_16_bit_int_be(std::vector<char>& buffer, std::uint32_t& offset) {
    std::uint16_t value = 0;
    for (std::uint32_t i = 0; i < 2; i++) {
        value = (value << 8) | static_cast<unsigned char>(buffer[offset + i]);
    }
    offset += 2;

    return value;
}

std::int8_t read_8_bit_int_be(std::vector<char>& buffer, std::uint32_t& offset) {
    std::int8_t value = static_cast<unsigned char>(buffer[offset]);
    offset++;

    return value;
}

std::string read_ascii_field(std::vector<char>& buffer, std::uint32_t& offset, std::uint32_t length) {
    std::string value;
    for (std::uint32_t i = 0; i < length; i++) {
        value += buffer[offset + i];  // raw char
    }
    offset += length;
    std::erase_if(value, [](unsigned char c) { return std::isspace(c); });

    return value;
}