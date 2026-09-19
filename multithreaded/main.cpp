#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <cstdint>
#include <chrono>
#include <variant>

#include "parser.h"
#include "book.h"
#include "messages.h"
#include "orders.h"
#include "aliases.h"

int main() {

	const std::string filename = "../data/100_mb_sample.bin";
	// const std::string filename = "../data/full_mb_sample.bin";
	std::unordered_map<StockIndex, OrderBook> globalOrderBook;  // book per stock

	// store all messages (to maintain order across types)
	std::vector<std::variant<AddOrder, ExecuteOrder, CancelOrder, DeleteOrder, ReplaceOrder>> messages;

	// map stocks to running price bounds and order counts
	std::unordered_map<StockIndex, StockStats> stockInfo;

	DataParser parser(filename, messages, stockInfo);

	auto start_parse_data = std::chrono::steady_clock::now();
	parser.getMessageAndStockData();  // parse itch messages
	auto end_parse_data = std::chrono::steady_clock::now();

	auto start_build_books = std::chrono::steady_clock::now();
	globalOrderBook.reserve(stockInfo.size());  // pre-allocate to num unique stocks
	for (const auto& [index, stats] : stockInfo) {
		globalOrderBook.emplace(index, OrderBook{ stats });  // store
	}
	auto end_build_books = std::chrono::steady_clock::now();

	// handle book operations from orders
	auto start_fill_books = std::chrono::steady_clock::now();
	for (const auto& action : messages) {

		// pass type from variant container
		std::visit(
			[&](const auto& order) {
				using T = std::decay_t<decltype(order)>;  // get type from decayed ref

				// handle global book with type checks (and disregard other branches)
				if constexpr (std::is_same_v<T, AddOrder>) {
					OrderBook& book = globalOrderBook.at(order.index);
					book.addOrder(order.id, order.side, order.shares, order.price);
				}
				else if constexpr (std::is_same_v<T, ExecuteOrder>) {
					OrderBook& book = globalOrderBook.at(order.index);
					book.executeOrder(order.id, order.sharesTraded);
				}
				else if constexpr (std::is_same_v<T, CancelOrder>) {
					OrderBook& book = globalOrderBook.at(order.index);
					book.cancelOrder(order.id, order.canceledShares);
				}
				else if constexpr (std::is_same_v<T, DeleteOrder>) {
					OrderBook& book = globalOrderBook.at(order.index);
					book.deleteOrder(order.id);
				}
				else if constexpr (std::is_same_v<T, ReplaceOrder>) {
					OrderBook& book = globalOrderBook.at(order.index);
					book.replaceOrder(order.oid, order.nid, order.newShares, order.newPrice);
				}
			},
			action);  // apply to iteration
	}
	auto end_fill_books = std::chrono::steady_clock::now();

	// record runtimes for each part
	auto parse_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_parse_data - start_parse_data);
	auto build_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_build_books - start_build_books);
	auto fill_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_fill_books - start_fill_books);

	std::cout << "Total Runtimes:\n"
		<< "ITCH DATA PARSE: " << parse_time_ms.count() << " ms\n"
		<< "BUILD EMPTY BOOKS: " << build_time_ms.count() << " ms\n"
		<< "SIMULATE ORDERS: " << fill_time_ms.count() << " ms\n";

	return 0;
}
