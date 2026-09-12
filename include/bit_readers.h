#ifndef BIT_READERS
#define BIT_READERS

#include <string>
#include <vector>
#include <cstdint>

using u64 = std::uint64_t;
using u32 = std::uint32_t;
using u16 = std::uint16_t;
using i32 = std::int32_t;
using i8 = std::int8_t;
using string = std::string;

u64 read_64_bit_int_be(std::vector<char>& buffer, u32& offset) {
    u64 value = 0;
    for (u32 i = 0; i < 8; i++) {
        value = (value << 8) | static_cast<unsigned char>(buffer[offset + i]);
    }
    offset += 8;

    return value;
}

u64 read_48_bit_int_be(std::vector<char>& buffer, u32& offset) {
    u64 value = 0;
    for (u32 i = 0; i < 6; i++) {
        value = (value << 8) | static_cast<unsigned char>(buffer[offset + i]);
    }
    offset += 6;

    return value;
}

i32 read_32_bit_int_be(std::vector<char>& buffer, u32& offset) {
    i32 value = 0;
    for (u32 i = 0; i < 4; i++) {
        value = (value << 8) | static_cast<unsigned char>(buffer[offset + i]);
    }
    offset += 4;

    return value;
}

u16 read_16_bit_int_be(std::vector<char>& buffer, u32& offset) {
    u16 value = 0;
    for (u32 i = 0; i < 2; i++) {
        value = (value << 8) | static_cast<unsigned char>(buffer[offset + i]);
    }
    offset += 2;

    return value;
}

i8 read_8_bit_int_be(std::vector<char>& buffer, u32& offset) {
    i8 value = static_cast<unsigned char>(buffer[offset]);
    offset += 1;

    return value;
}

string read_ascii_field(std::vector<char>& buffer, u32& offset, u32 length) {
    string value;
    for (u32 i = 0; i < length; i++) {
        value += buffer[offset + i];  // raw char
    }
    offset += length;
    std::erase_if(value, [](unsigned char c) { return std::isspace(c); });

    return value;
}

#endif