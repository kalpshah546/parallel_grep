#include "searcher.h"
#include "threadpool.h"
#include "benchmark.h"
#include <iostream>
#include <cstdlib>
#include <string>

int main(int argc, char* argv[])
{
    if (argc >= 2 && std::string(argv[1]) == "--benchmark") {
        if (argc < 4) {
            std::cerr << "Usage: " << argv[0]
                      << " --benchmark <directory> <keyword>\n";
            return 1;
        }

        std::string directory = argv[2];
        std::string keyword = argv[3];

        if (!std::filesystem::exists(directory)) {
            std::cerr << "Error: directory '" << directory << "' does not exist.\n";
            return 1;
        }

        std::cout << "Benchmarking '" << directory << "' for \""
                  << keyword << "\" (workers: 1, 2, 4, 8)...\n\n";

        runWorkerSweepBenchmark(directory, keyword);
        return 0;
    }

    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <directory> <keyword> [thread_count]\n";
        std::cerr << "       " << argv[0] << " --benchmark <directory> <keyword>\n";
        std::cerr << "  directory    Path to search (searched recursively)\n";
        std::cerr << "  keyword      Text pattern to search for in file contents\n";
        std::cerr << "  thread_count Optional. Number of worker threads (default: 8)\n";
        return 1;
    }

    std::string directory = argv[1];
    std::string keyword = argv[2];

    int threadCount = 8;
    if (argc >= 4) {
        threadCount = std::atoi(argv[3]);
        if (threadCount <= 0) {
            std::cerr << "Error: thread_count must be a positive integer.\n";
            return 1;
        }
    }

    if (!std::filesystem::exists(directory)) {
        std::cerr << "Error: directory '" << directory << "' does not exist.\n";
        return 1;
    }

    std::cout << "Searching '" << directory << "' for \"" << keyword
              << "\" using " << threadCount << " threads...\n\n";

    ThreadPool pool(threadCount);
    Searcher searcher(pool);

    runBenchmark(searcher, directory, keyword, threadCount);

    return 0;
}