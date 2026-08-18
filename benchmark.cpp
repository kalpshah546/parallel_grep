#include "benchmark.h"
#include "searcher.h"
#include "threadpool.h"
#include <iostream>
#include <chrono>
#include <vector>

void runBenchmark(
    Searcher& searcher,
    const std::filesystem::path& directory,
    const std::string& keyword)
{
    auto start_seq = std::chrono::high_resolution_clock::now();
    auto seqResults = searcher.sequentialSearch(directory, keyword);
    auto end_seq = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> seqTime = end_seq - start_seq;

    auto start_par = std::chrono::high_resolution_clock::now();
    auto parResults = searcher.parallelSearch(directory, keyword);
    auto end_par = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> parTime = end_par - start_par;

    std::cout << "Sequential matches: " << seqResults.size() << "\n";
    std::cout << "Parallel matches:   " << parResults.size() << "\n";

    if (seqResults.size() != parResults.size()) {
        std::cout << "WARNING: match counts differ — check for bugs.\n";
    }

    std::cout << "Sequential time: " << seqTime.count() << " ms\n";
    std::cout << "Parallel time:   " << parTime.count() << " ms\n";
    std::cout << "Speedup: " << (seqTime.count() / parTime.count()) << "\n";
}

void runWorkerSweepBenchmark(
    const std::filesystem::path& directory,
    const std::string& keyword)
{
    ThreadPool seqPool(1);
    Searcher seqSearcher(seqPool);

    auto start_seq = std::chrono::high_resolution_clock::now();
    auto seqResults = seqSearcher.sequentialSearch(directory, keyword);
    auto end_seq = std::chrono::high_resolution_clock::now();
    const double seqTimeMs =
        std::chrono::duration<double, std::milli>(end_seq - start_seq).count();

    std::cout << "Sequential baseline: " << seqTimeMs << " ms ("
              << seqResults.size() << " matches)\n\n";

    const std::vector<int> workerCounts = {1, 2, 4, 8};

    for (int workers : workerCounts) {
        ThreadPool pool(static_cast<size_t>(workers));
        Searcher searcher(pool);

        auto start = std::chrono::high_resolution_clock::now();
        auto results = searcher.parallelSearch(directory, keyword);
        auto end = std::chrono::high_resolution_clock::now();
        const double parTimeMs =
            std::chrono::duration<double, std::milli>(end - start).count();

        if (results.size() != seqResults.size()) {
            std::cout << "WARNING: match count differs at " << workers
                      << " workers — check for bugs.\n";
        }

        std::cout << "Workers: " << workers << "\n";
        std::cout << "Time: " << parTimeMs << " ms\n";
        std::cout << "Speedup: " << (seqTimeMs / parTimeMs) << "\n\n";
    }
}