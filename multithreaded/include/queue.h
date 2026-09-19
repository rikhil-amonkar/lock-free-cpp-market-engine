#include <queue>
#include <mutex>
#include <variant>
#include <condition_variable>
#include <atomic>

#include "messages.h"
#include "aliases.h"

class ThreadSafeSharedQueue {
    std::queue<orderTypes> sharedQueue;
    mutable std::mutex mtx;
    std::condition_variable cv;
    std::atomic<bool> done = false;

public:
    ThreadSafeSharedQueue() = default;  // explicit default constructor

    void push(const orderTypes& order);

    bool pop(orderTypes& output);

    bool is_empty();

    void set_done();
};