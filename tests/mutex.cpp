#include <iostream>
#include <thread>
#include <mutex>

std::mutex mtx;

int total = 0;

void task() {
    mtx.lock();  // locks other threads from this task

    for (int i = 0; i < 1000000; i++) {
        total++;
    }

    mtx.unlock();  // unlock rest after one thread finishes
}

/*
Without a mutex locking and unlocking the threads, since the data is shared, the function above results in unpredictable values. In the case above, instead of producing 2000000 for the total increments during the thread call to both t1 and t2 (since 1000000 + 1000000), the value gets corrupted during threads since it is shared and becomes something like 1009536 or another number.

To prevent this we use mutexes. When the proper lock and unlock techniques are used on the task being shared between two threads, the proper value of 2000000 is produced.
*/

int main() {
    std::thread t1(task);
    std::thread t2(task);

    t1.join();
    t2.join();

    std::cout << "Threads t1 and t2 resulted in " << total << "\n";

    return 0;
}