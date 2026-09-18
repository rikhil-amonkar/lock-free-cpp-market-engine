#include <iostream>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <format>
#include <thread>

class ThreadSafeQueue {
    std::queue<int> q;
    mutable std::mutex mtx;  // allow lock/unlock for const funcs

public:
    ThreadSafeQueue() = default;  // generate default constructor

    void push_item(const int& value) {
        std::lock_guard<std::mutex> lock(mtx);  // lock other threads

        q.push(value);
    }

    bool try_pop() {
        std::lock_guard<std::mutex> lock(mtx);  // lock other threads

        if (q.empty()) {  // check for empty queue
            return false;
        }
        int item = std::move(q.front());
        q.pop();
        return true;
    }

    bool is_empty() const {
        std::lock_guard<std::mutex> lock(mtx);

        return q.empty();
    }

    void display() {
        std::lock_guard<std::mutex> lock(mtx);

        std::queue<int> temp = q;
        while (!temp.empty()) {
            std::cout << temp.front() << " ";
            temp.pop();
        }
        std::cout << "\n";
    }
};

void producer(ThreadSafeQueue& q, int& pushed) {
    for (int i = 0; i < 10000; i++) {
        q.push_item(i);
        pushed++;
    }
}

void consumer(ThreadSafeQueue& q, int& popped) {
    while (popped < 10000) {
        if (q.try_pop()) {
            popped++;
        }
    }
}

int main() {
    ThreadSafeQueue q;  // FIFO
    int pushed = 0;
    int popped = 0;

    std::thread a(producer, std::ref(q), std::ref(pushed));
    std::thread b(consumer, std::ref(q), std::ref(popped));

    a.join();
    b.join();

    std::cout << std::format("Pushed {} items, Popped {} items\n", pushed, popped);
}