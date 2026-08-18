# Parallel Grep — Multithreaded File Search in C++

A command-line tool that recursively scans a directory tree for a keyword and distributes the file-reading work across a custom thread pool. The project includes both a normal search mode and a benchmark mode to compare sequential and parallel performance.

## Why this project

When a directory contains hundreds or thousands of files, a single-threaded scan spends most of its time waiting on disk I/O instead of using the CPU. This project spreads the work across worker threads so multiple files can be searched concurrently.

The thread pool is implemented from scratch with a task queue, worker threads, and synchronization primitives rather than using a higher-level abstraction like `std::async`.

## Features

- Recursive directory search for a keyword substring in file contents
- Custom thread pool implementation with a task queue and `std::future` result handling
- Sequential and parallel search paths with correctness checking
- Built-in benchmark runner for comparing workers across a sweep
- Configurable worker count from the command line
- Binary-file filtering by extension
- Cancellation-aware file search support for stopping work early
- CMake-based build configuration

## Project layout

```text
main.cpp           → CLI entry point, argument parsing, dispatch
threadpool.h/.cpp  → Generic thread pool implementation
searcher.h/.cpp    → File scanning, recursive traversal, sequential/parallel logic
benchmark.h/.cpp   → Timing and speedup reporting
metrics.h          → Common metrics/reporting helpers
cancellation_test.cpp → Cancellation-path validation
threadpool_test.cpp → Thread-pool validation
```

## Build

### With CMake (recommended)

```bash
cmake -S . -B build
cmake --build build
```

The executable is created at `./build/parallel_grep`.

## Usage

```bash
./build/parallel_grep <directory> <keyword> [thread_count]
./build/parallel_grep --benchmark <directory> <keyword>
```

- `directory` — root directory to search recursively
- `keyword` — text to find within file contents
- `thread_count` — optional, defaults to 8

Examples:

```bash
./build/parallel_grep ./test_data ERROR 8
./build/parallel_grep --benchmark ./test_data ERROR
```

## Sample data

The repository includes a sample tree under `test_data/` for quick benchmarking and validation. The benchmark compares the sequential baseline against parallel runs using worker counts of 1, 2, 4, and 8.

## Sample output

```text
Searching './test_data' for 'ERROR' using 8 threads...

========== Results ==========
Files processed: 2000
Matches: 666
Workers: 8
Time: 211.7 ms
Speedup: 3.98x
==============================
```

Actual result values vary by machine, input size, and hardware.

## Design notes

- Why a custom thread pool instead of `std::async`: the project is intentionally educational. Building the queue, worker loop, and future handoff from scratch makes the synchronization model visible and explainable.
- Why check `sequential matches == parallel matches`: a fast, incorrect result is worse than a slow, correct one. This comparison guards against dropped tasks or race conditions.
- Why exclude binary files: searching raw binary data for a text keyword is usually meaningless and expensive. Extension-based filtering keeps the tool focused on text files.
- Why include a benchmark sweep: it makes the behavior more transparent than a single timing number and highlights how parallelism changes with worker count.

