#ifndef BENCHMARK_H
#define BENCHMARK_H

#include "searcher.h"
#include <filesystem>
#include <string>

void runBenchmark(
    Searcher& searcher,
    const std::filesystem::path& directory,
    const std::string& keyword);

#endif