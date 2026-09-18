#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>

// mutex to block threads
std::mutex mtx;
std::condition_variable cv;

// flag to avoid spurios wakeup
bool data_ready = false;

// producer function working as sender
void producer() {

    // Simulate data production
    std::this_thread::sleep_for(std::chrono::seconds(3));

    // mutex locks immediately with guard 
    std::lock_guard<std::mutex> lock(mtx);

    // variable to avoid spurious wakeup
    data_ready = true;

    // logging notification to console
    std::cout << "Data Produced!\n";

    // notify consumer when done
    cv.notify_one();
}

// consumer that will receive what producer has produced
void consumer() {

    // mutex needs manual locking/unlocking
    std::unique_lock<std::mutex> lock(mtx);

    std::cout << "Starting...\n";
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "Finished!\n";

    // unlocks mutex, waits until task, relocks after
    cv.wait(lock, [] { return data_ready; });

    std::cout << "Data consumed!\n";
}

/*
If using a condition variable's wait method, it is required that you use the unique_lock instead of the lock_guard. This is because condition variables needs to unlock the mutex while it waits, but the lock_guard does not allow manual unlocking.
*/

// drive code
int main() {
    std::thread consumer_thread(consumer);
    std::thread producer_thread(producer);

    consumer_thread.join();
    producer_thread.join();

    return 0;
}