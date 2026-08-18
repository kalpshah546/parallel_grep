#include "searcher.h"
#include <fstream>
#include <iostream>
#include <unordered_set>

Searcher::Searcher(ThreadPool& pool, std::atomic<bool>* cancelled)
    : pool(pool), cancelled(cancelled) {}

bool isBinaryFile(const std::filesystem::path& file)
{
    static const std::unordered_set<std::string> binaryExtensions = {
        ".exe", ".dll", ".obj", ".o", ".bin", ".png", ".jpg", ".jpeg",
        ".gif", ".bmp", ".ico", ".pdf", ".zip", ".rar", ".7z", ".gz",
        ".mp3", ".mp4", ".avi", ".mov", ".class", ".pyc", ".so"
    };

    std::string ext = file.extension().string();
    return binaryExtensions.count(ext) > 0;
}
std::vector<SearchResult> Searcher::searchFile(
    const std::filesystem::path& file,
    const std::string& keyword)
{
    std::vector<SearchResult> results;
    
    std::ifstream infile(file);
    if (!infile.is_open()) {
        return results;
    }

    std::string line;
    int lineNumber = 0;

    while (std::getline(infile, line)) {
        if (isCancelled()) {
            return results;
        }
        lineNumber++;
        if (line.find(keyword) != std::string::npos) {
            results.push_back({file.string(), lineNumber, line});
        }
    }

    return results;
}

std::vector<SearchResult> Searcher::sequentialSearch(
    const std::filesystem::path& directory,
    const std::string& keyword)
{
    std::vector<SearchResult> allResults;

    for (const auto& entry : std::filesystem::recursive_directory_iterator(directory)) {
        if (isCancelled()) {
            break;
        }
        if (entry.is_regular_file() && !isBinaryFile(entry.path())) {
            auto fileResults = searchFile(entry.path(), keyword);
            allResults.insert(allResults.end(), fileResults.begin(), fileResults.end());
        }
    }

    return allResults;
}

std::vector<SearchResult> Searcher::parallelSearch(
    const std::filesystem::path& directory,
    const std::string& keyword)
{
    std::vector<std::filesystem::path> files;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(directory)) {
        if (isCancelled()) {
            break;
        }
        if (entry.is_regular_file() && !isBinaryFile(entry.path())) {
            files.push_back(entry.path());
        }
    }

    std::vector<std::future<std::vector<SearchResult>>> futures;
    futures.reserve(files.size());

    for (const auto& file : files) {
        if (isCancelled()) {
            break;
        }
        futures.push_back(
            pool.submit(
                [this, file, keyword]() {
                    return searchFile(file, keyword);
                }
            )
        );
    }

    std::vector<SearchResult> allResults;
    for (auto& fut : futures) {
        auto fileResults = fut.get();
        allResults.insert(allResults.end(), fileResults.begin(), fileResults.end());
    }

    return allResults;
}