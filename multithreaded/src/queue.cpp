#include <queue>
#include <mutex>
#include <variant>
#include <condition_variable>

#include "messages.h"
#include "aliases.h"
#include "queue.h"

void ThreadSafeSharedQueue::push(const orderTypes& order) {
    std::lock_guard<std::mutex> lock(mtx);  // lock threads (auto)
    sharedQueue.push(order);
    cv.notify_one();  // tell pop of new data
}

bool ThreadSafeSharedQueue::pop(orderTypes& output) {
    std::unique_lock<std::mutex> lock(mtx);  // lock threads (manual)
    cv.wait(lock, [this] { return !sharedQueue.empty() || done; });  // wait until an item is pushed or empty
    if (sharedQueue.empty()) {
        return false;
    }
    output = std::move(sharedQueue.front());
    sharedQueue.pop();
    return true;
}

bool ThreadSafeSharedQueue::is_empty() {
    std::lock_guard<std::mutex> lock(mtx);  // lock threads (auto)
    return sharedQueue.empty();
}

void ThreadSafeSharedQueue::set_done() {  // after parse ends
    std::lock_guard<std::mutex> lock(mtx);
    done = true;
    cv.notify_all();  // tell every thread data has ended
}