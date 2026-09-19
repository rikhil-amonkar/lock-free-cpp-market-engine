#ifndef TYPE_ALIASES
#define TYPE_ALIASES

#include <string>
#include <cstdint>

using StockIndex = std::uint16_t;
using TrackNumber = std::uint16_t;
using Timestamp = std::uint64_t;
using MatchId = std::uint64_t;
using Ticker = std::string;
using MPID = std::string;
using EventCode = char;
using Printable = char;

using OrderId = std::uint64_t;
using Side = char;
using Quantity = std::int32_t;
using Price = std::int32_t;
using PoolIndex = std::uint32_t;
using StockIndex = std::uint16_t;

#endif