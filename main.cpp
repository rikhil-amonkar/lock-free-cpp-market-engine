#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <cstdint>

#include "./include/parser.h"
#include "./include/book.h"
#include "./include/messages.h"
#include "./include/orders.h"
#include "./include/aliases.h"

int main() {

	const std::string filename = "./data/sample.bin";
	// const std::string filename = "./data/20190730.BX_ITCH_50.bin";
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
	parser.getMessageAndStockData();  // parse itch messages

	globalOrderBook.reserve(stockInfo.size());  // pre-allocate to num unique stocks
	for (const auto& [index, stats] : stockInfo) {
		globalOrderBook.emplace(index, OrderBook{ stats });  // store
	}

	// handle book operations from orders
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

	// std::uint32_t iterations = 0;
	// for (auto& [index, book] : globalOrderBook) {
	// 	book.printAllOrders();
	// 	if (iterations == 10) {
	// 		break;
	// 	}
	// 	iterations++;
	// }

	return 0;
}
