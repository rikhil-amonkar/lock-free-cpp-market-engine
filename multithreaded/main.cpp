#include <iostream>
#include <unordered_map>
#include <string>
#include <cstdint>
#include <chrono>
#include <variant>
#include <thread>

#include "parser.h"
#include "book.h"
#include "messages.h"
#include "orders.h"
#include "aliases.h"
#include "queue.h"
#include "buffer.h"

void producer(const std::vector<char>& buffer, ThreadSafeSharedQueue& queue) {
	DataParser parser(buffer, queue);
	parser.processDataStream();  // parse itch messages
	queue.set_done();  // producer flags as complete
}

void consumer(std::unordered_map<StockIndex, OrderBook>& globalOrderBook, ThreadSafeSharedQueue& queue) {
	while (true) {  // handle book operations
		OrderTypes action{ AddOrder{ 0, 0, 'B', 0, 0 } };  // use placeholder for variant constructor
		if (!queue.pop(action)) {
			break;
		}

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
}

int main() {

	// const std::string filename = "../data/100_mb_sample.bin";
	const std::string filename = "../data/full_mb_sample.bin";
	std::unordered_map<StockIndex, OrderBook> globalOrderBook;  // book per stock

	// map stocks to running price bounds and order counts
	std::unordered_map<StockIndex, StockStats> stockInfo;

	// thread safe queue for producer/consumer shared use
	ThreadSafeSharedQueue queue;

	// store file bytes into a buffer storage
	auto start_init_buffer = std::chrono::steady_clock::now();
	ByteBuffer storage(filename, stockInfo);
	const std::vector<char> buffer = storage.createBuffer();
	auto end_init_buffer = std::chrono::steady_clock::now();

	auto start_build_books = std::chrono::steady_clock::now();
	globalOrderBook.reserve(stockInfo.size());  // pre-allocate to num unique stocks
	for (const auto& [index, stats] : stockInfo) {
		globalOrderBook.emplace(index, OrderBook{ stats });  // store
	}
	auto end_build_books = std::chrono::steady_clock::now();

	// starts seperate thread work on queue
	std::thread t1(producer, std::ref(buffer), std::ref(queue));  // push to queue
	std::thread t2(consumer, std::ref(globalOrderBook), std::ref(queue));  // pop from queue

	auto start_parse_and_fill_multithread = std::chrono::steady_clock::now();
	// make the main thread wait for both to finish
	t1.join();
	t2.join();
	auto end_parse_and_fill_multithread = std::chrono::steady_clock::now();

	// for (auto& [index, book] : globalOrderBook) {
	// 	book.printAllOrders();
	// }

	// record runtimes for each part
	auto parse_to_buffer_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_init_buffer - start_init_buffer);
	auto build_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_build_books - start_build_books);
	auto multithread_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_parse_and_fill_multithread - start_parse_and_fill_multithread);

	std::cout << "Total Runtimes:\n"
		<< "INIT BUFFER FILL: " << parse_to_buffer_ms.count() << " ms\n"
		<< "BUILD EMPTY BOOKS: " << build_time_ms.count() << "ms\n"
		<< "SIMULATE ORDERS (MULTITHREAD): " << multithread_time_ms.count() << " ms\n";

	return 0;
}
