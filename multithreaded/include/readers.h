#ifndef BIT_READERS
#define BIT_READERS

#include <string>
#include <vector>
#include <cstdint>

std::uint64_t read_64_bit_int_be(const std::vector<char>& buffer, std::uint32_t& offset);

std::uint64_t read_48_bit_int_be(const std::vector<char>& buffer, std::uint32_t& offset);

std::int32_t read_32_bit_int_be(const std::vector<char>& buffer, std::uint32_t& offset);

std::uint16_t read_16_bit_int_be(const std::vector<char>& buffer, std::uint32_t& offset);

std::int8_t read_8_bit_int_be(const std::vector<char>& buffer, std::uint32_t& offset);

std::string read_ascii_field(const std::vector<char>& buffer, std::uint32_t& offset, std::uint32_t length);

#endif