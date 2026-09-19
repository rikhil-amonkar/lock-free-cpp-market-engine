#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <cstdint>
#include <chrono>

#include "parser.h"
#include "book.h"
#include "messages.h"
#include "orders.h"
#include "aliases.h"

int main() {

	const std::string filename = "../data/100_mb_sample.bin";
	// const std::string filename = "../data/full_mb_sample.bin";
	std::unordered_map<StockIndex, OrderBook> globalOrderBook;  // book per stock

	// storage vectors for messages
	std::vector<SystemEvent> systemEvents;
	std::vector<AddOrder> addOrders;
	std::vector<AddOrderMPID> addOrdersMPID;
	std::vector<OrderExecuted> executeOrders;
	std::vector<OrderExecutedPrice> executeOrdersPrice;
	std::vector<OrderCancel> cancelOrders;
	std::vector<OrderDelete> deleteOrders;
	std::vector<OrderReplace> replaceOrders;

	// store all messages (to maintain order across types)
	std::vector<BaseMessage> allMessages;

	// map stocks to running price bounds and order counts
	std::unordered_map<StockIndex, StockStats> stockInfo;

	DataParser parser(
		filename, systemEvents, addOrders, addOrdersMPID,
		executeOrders, executeOrdersPrice, cancelOrders,
		deleteOrders, replaceOrders, allMessages, stockInfo
	);

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
	for (const auto& [messageType, typeIndex] : allMessages) {
		if (messageType == "AddOrder") {
			AddOrder action = addOrders[typeIndex];
			OrderBook& book = globalOrderBook.at(action.stockIndex);
			book.addOrder(action.orderId, action.side, action.shares, action.price);
		}
		else if (messageType == "AddOrderMPID") {
			AddOrderMPID action = addOrdersMPID[typeIndex];
			OrderBook& book = globalOrderBook.at(action.stockIndex);
			book.addOrder(action.orderId, action.side, action.shares, action.price);
		}
		else if (messageType == "OrderExecuted") {
			OrderExecuted action = executeOrders[typeIndex];
			OrderBook& book = globalOrderBook.at(action.stockIndex);
			book.executeOrder(action.orderId, action.sharesTraded);
		}
		else if (messageType == "OrderExecutedPrice") {
			OrderExecutedPrice action = executeOrdersPrice[typeIndex];
			OrderBook& book = globalOrderBook.at(action.stockIndex);
			book.executeOrder(action.orderId, action.sharesTraded);
		}
		else if (messageType == "OrderCancel") {
			OrderCancel action = cancelOrders[typeIndex];
			OrderBook& book = globalOrderBook.at(action.stockIndex);
			book.cancelOrder(action.orderId, action.canceledShares);
		}
		else if (messageType == "OrderDelete") {
			OrderDelete action = deleteOrders[typeIndex];
			OrderBook& book = globalOrderBook.at(action.stockIndex);
			book.deleteOrder(action.orderId);
		}
		else if (messageType == "OrderReplace") {
			OrderReplace action = replaceOrders[typeIndex];
			OrderBook& book = globalOrderBook.at(action.stockIndex);
			book.replaceOrder(action.initOrderId, action.newOrderId, action.newShares, action.newPrice);
		}
		else {
			continue;  // skip system events
		}
	}
	auto end_fill_books = std::chrono::steady_clock::now();

	for (auto& [index, book] : globalOrderBook) {
		book.printAllOrders();
	}

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
