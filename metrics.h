#ifndef METRICS_H
#define METRICS_H

#include <cstddef>
#include <iostream>

struct SearchMetrics
{
    std::size_t filesProcessed = 0;
    std::size_t matchesFound = 0;
    std::size_t tasksSubmitted = 0;
    std::size_t tasksCompleted = 0;
};

inline void printResultsSummary(
    const SearchMetrics& metrics,
    int workers,
    double timeMs,
    double speedup)
{
    std::cout << "========== Results ==========\n";
    std::cout << "Files processed: " << metrics.filesProcessed << "\n";
    std::cout << "Matches: " << metrics.matchesFound << "\n";
    std::cout << "Workers: " << workers << "\n";
    std::cout << "Time: " << timeMs << " ms\n";
    std::cout << "Speedup: " << speedup << "x\n";
    if (metrics.tasksSubmitted > 0) {
        std::cout << "Tasks submitted: " << metrics.tasksSubmitted << "\n";
        std::cout << "Tasks completed: " << metrics.tasksCompleted << "\n";
    }
    std::cout << "==============================\n";
}

#endif
