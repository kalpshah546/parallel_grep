#ifndef SEARCHER_H
#define SEARCHER_H
#include <stdio.h>
#include <string>
#include <filesystem>
#include <atomic>
#include "threadpool.h"
#include "metrics.h"

struct SearchResult
{
    std::string filename;
    int lineNumber;
    std::string line;
};

class Searcher
{
private:
    ThreadPool& pool;
    std::atomic<bool>* cancelled;

    bool isCancelled() const
    {
        return cancelled && cancelled->load(std::memory_order_relaxed);
    }

public:
    Searcher(ThreadPool& pool, std::atomic<bool>* cancelled = nullptr);

    std::vector<SearchResult> searchFile(
        const std::filesystem::path& file,
        const std::string& keyword);

    std::vector<SearchResult> parallelSearch(
        const std::filesystem::path& directory,
        const std::string& keyword,
        SearchMetrics* metrics = nullptr);

    std::vector<SearchResult> sequentialSearch(
        const std::filesystem::path& directory,
        const std::string& keyword,
        SearchMetrics* metrics = nullptr);
};
#endif