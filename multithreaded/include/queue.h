#ifndef QUEUE_CLASS
#define QUEUE_CLASS

#include <queue>
#include <mutex>
#include <variant>
#include <condition_variable>
#include <atomic>

#include "messages.h"
#include "aliases.h"

class ThreadSafeSharedQueue {
    std::queue<OrderTypes> sharedQueue;
    mutable std::mutex mtx;
    std::condition_variable cv;
    std::atomic<bool> done = false;

public:
    ThreadSafeSharedQueue() = default;  // explicit default constructor

    void push(const OrderTypes& order);

    bool pop(OrderTypes& output);

    bool is_empty();

    void set_done();
};

#endif