#include <iostream>
#include <chrono>
#include <thread>

void task() {
    std::cout << "Starting a task on this thread...\n";
    std::this_thread::sleep_for(std::chrono::seconds(3));
    std::cout << "About to finish...\n";  // won't run if detached since the main thread ends first and terminates all other threads
}

int main() {
    std::thread t(task);  // create thread

    if (t.joinable()) {  // will be true (since it has not been called)
        std::cout << "Thread " << t.get_id() << " is an ACTIVE thread.\n";
    }

    t.join();  // main waits till task complete

    // t.detach();  // will run independently

    std::cout << "Thread " << t.get_id() << " has finished!\n";  // waits for thread to finish

    if (!t.joinable()) {  // now will be terminated
        std::cout << "This thread has been KILLED.\n";
    }

    return 0;
}