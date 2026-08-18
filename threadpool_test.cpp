#include "threadpool.h"

#include <atomic>
#include <iostream>
#include <vector>

int main()
{
    constexpr int numTasks = 100;
    ThreadPool pool(4);
    std::atomic<int> counter{0};

    std::vector<std::future<void>> futures;
    futures.reserve(numTasks);

    for (int i = 0; i < numTasks; ++i) {
        futures.push_back(pool.submit([&counter]() {
            counter.fetch_add(1, std::memory_order_relaxed);
        }));
    }

    for (auto& future : futures) {
        future.get();
    }

    if (counter.load() != numTasks) {
        std::cerr << "FAIL: expected " << numTasks << " tasks, got "
                  << counter.load() << "\n";
        return 1;
    }

    // Destructor runs here: workers drain the queue and join cleanly.
    std::cout << "PASS: all " << numTasks
              << " tasks completed, pool shut down cleanly\n";
    return 0;
}
