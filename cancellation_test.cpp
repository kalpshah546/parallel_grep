#include "searcher.h"
#include "threadpool.h"

#include <atomic>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <thread>

int main(int argc, char* argv[])
{
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <directory>\n";
        return 1;
    }

    const std::filesystem::path directory = argv[1];
    if (!std::filesystem::exists(directory)) {
        std::cerr << "Error: directory '" << directory << "' does not exist.\n";
        return 1;
    }

    ThreadPool fullPool(4);
    Searcher fullSearcher(fullPool);
    const auto fullResults = fullSearcher.parallelSearch(directory, "ERROR");
    const std::size_t fullCount = fullResults.size();

    std::atomic<bool> cancelled{false};
    ThreadPool pool(4);
    Searcher searcher(pool, &cancelled);

    std::size_t partialCount = 0;
    auto searchStart = std::chrono::steady_clock::now();

    std::thread searchThread([&]() {
        partialCount = searcher.parallelSearch(directory, "ERROR").size();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    cancelled.store(true, std::memory_order_relaxed);

    searchThread.join();
    const auto elapsedMs = std::chrono::duration<double, std::milli>(
        std::chrono::steady_clock::now() - searchStart).count();

    if (partialCount >= fullCount) {
        std::cerr << "FAIL: cancelled search returned " << partialCount
                  << " matches, expected fewer than " << fullCount << "\n";
        return 1;
    }

    std::cout << "PASS: cancelled search stopped early ("
              << partialCount << " of " << fullCount
              << " matches) in " << elapsedMs << " ms\n";
    return 0;
}
