#ifndef SEARCHER_H
#define SEARCHER_H
#include <stdio.h>
#include <string>
#include <filesystem>
#include "threadpool.h"

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

public:
    Searcher(ThreadPool& pool);

    std::vector<SearchResult> searchFile(
        const std::filesystem::path& file,
        const std::string& keyword);

    std::vector<SearchResult> parallelSearch(
        const std::filesystem::path& directory,
        const std::string& keyword);

    std::vector<SearchResult> sequentialSearch(
        const std::filesystem::path& directory,
        const std::string& keyword);
};
#endif