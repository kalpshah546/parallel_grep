#include "benchmark.h"
#include <iostream>
#include <chrono>

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