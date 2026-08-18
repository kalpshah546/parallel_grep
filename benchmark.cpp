#include "benchmark.h"
#include "searcher.h"
#include "threadpool.h"
#include "metrics.h"
#include <iostream>
#include <chrono>
#include <vector>

void runBenchmark(
    Searcher& searcher,
    const std::filesystem::path& directory,
    const std::string& keyword,
    int workers)
{
    SearchMetrics seqMetrics;
    auto start_seq = std::chrono::high_resolution_clock::now();
    auto seqResults = searcher.sequentialSearch(directory, keyword, &seqMetrics);
    auto end_seq = std::chrono::high_resolution_clock::now();
    const double seqTimeMs =
        std::chrono::duration<double, std::milli>(end_seq - start_seq).count();

    SearchMetrics parMetrics;
    auto start_par = std::chrono::high_resolution_clock::now();
    auto parResults = searcher.parallelSearch(directory, keyword, &parMetrics);
    auto end_par = std::chrono::high_resolution_clock::now();
    const double parTimeMs =
        std::chrono::duration<double, std::milli>(end_par - start_par).count();

    if (seqResults.size() != parResults.size()) {
        std::cout << "WARNING: match counts differ — check for bugs.\n\n";
    }

    const double speedup = seqTimeMs / parTimeMs;
    printResultsSummary(parMetrics, workers, parTimeMs, speedup);
}

void runWorkerSweepBenchmark(
    const std::filesystem::path& directory,
    const std::string& keyword)
{
    ThreadPool seqPool(1);
    Searcher seqSearcher(seqPool);

    SearchMetrics seqMetrics;
    auto start_seq = std::chrono::high_resolution_clock::now();
    auto seqResults = seqSearcher.sequentialSearch(directory, keyword, &seqMetrics);
    auto end_seq = std::chrono::high_resolution_clock::now();
    const double seqTimeMs =
        std::chrono::duration<double, std::milli>(end_seq - start_seq).count();

    std::cout << "Sequential baseline: " << seqTimeMs << " ms ("
              << seqResults.size() << " matches, "
              << seqMetrics.filesProcessed << " files)\n\n";

    const std::vector<int> workerCounts = {1, 2, 4, 8};

    for (int workers : workerCounts) {
        ThreadPool pool(static_cast<size_t>(workers));
        Searcher searcher(pool);

        SearchMetrics metrics;
        auto start = std::chrono::high_resolution_clock::now();
        auto results = searcher.parallelSearch(directory, keyword, &metrics);
        auto end = std::chrono::high_resolution_clock::now();
        const double parTimeMs =
            std::chrono::duration<double, std::milli>(end - start).count();

        if (results.size() != seqResults.size()) {
            std::cout << "WARNING: match count differs at " << workers
                      << " workers — check for bugs.\n\n";
        }

        printResultsSummary(metrics, workers, parTimeMs, seqTimeMs / parTimeMs);
        std::cout << "\n";
    }
}