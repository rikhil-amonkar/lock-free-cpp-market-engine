#include <iostream>
#include <unordered_map>
#include <string>
#include <cstdint>

#include "./include/parser.h"
#include "./include/book.h"

using StockIndex = std::uint16_t;

int main() {

	const std::string filename = "./src/sample.bin";
	std::unordered_map<StockIndex, OrderBook> globalOrderBook;  // book per stock

	DataParser parser(filename);  // parse itch messages
	auto [allMessages, stockInfo] = parser.getMessageAndStockData();

	globalOrderBook.reserve(stockInfo.size());  // pre-allocate to num unique stocks
	for (const auto& [index, stats] : stockInfo) {
		globalOrderBook.emplace(index, OrderBook{ stats });  // store
	}

	return 0;
}
