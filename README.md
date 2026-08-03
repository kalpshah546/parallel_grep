# Parallel Grep — Multithreaded File Content Search (C++)

A command-line tool that searches file contents for a keyword across a directory tree, using a custom-built thread pool to parallelize the search across multiple worker threads. Includes a built-in benchmark comparing sequential vs parallel search performance.

## Why this project

Sequential directory search wastes CPU: when searching hundreds or thousands of files, most of the time is spent waiting on I/O for one file at a time while other CPU cores sit idle. This project distributes file search work across a pool of worker threads, so multiple files are read and searched concurrently — with the actual thread pool built from scratch (not `std::async` or a library) to understand what's happening underneath.

## Features

- Recursive directory search for a keyword substring in file contents
- Custom thread pool implementation (task queue, condition variable synchronization, `std::future`-based result retrieval)
- Sequential vs parallel benchmarking built in, with timing and speedup reported per run
- Configurable thread count via CLI argument
- Skips binary files (images, executables, archives, etc.) by extension
- CMake build support

## Architecture

```
main.cpp        → CLI entry point, argument parsing, wiring
threadpool.h    → Generic thread pool: task queue + worker threads + future-based results
searcher.h/.cpp → Directory traversal, per-file keyword search (sequential + parallel paths)
benchmark.h/.cpp→ Runs both search modes, times them, reports speedup
```

The thread pool is generic — it accepts any callable via a variadic template (`submit(F&&, Args&&...)`) and returns a `std::future` for the result, so it isn't tied to the search use case specifically.

## Build

### With CMake (recommended)

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

### Direct compilation

```bash
g++ -std=c++17 -O2 -pthread main.cpp searcher.cpp benchmark.cpp threadpool.cpp -o parallel_grep
```

## Usage

```bash
./parallel_grep <directory> <keyword> [thread_count]
```

- `directory` — path to search (recursive)
- `keyword` — substring to search for in file contents
- `thread_count` — optional, defaults to 8

Example:
```bash
./parallel_grep ./test_data ERROR 8
```

## Generating test data

Sample test files aren't included in the repo (generated data doesn't belong in version control). Generate your own:

```powershell
./generate_test_data.ps1
```

This creates 2000 sample `.txt` files of varying size, with "ERROR" seeded into roughly every 3rd file — giving a known expected match count to verify correctness against.

## Sample results

```
Sequential matches: 666
Parallel matches:   666
Sequential time: 842.3 ms
Parallel time:   211.7 ms
Speedup: 3.98x
```

(Actual numbers vary by machine, file count/size, and thread count. Run your own benchmark — see below.)

## Design notes

- **Why a custom thread pool instead of `std::async`:** built from scratch to understand and be able to explain the underlying mechanics — task queue management, condition variable signaling, and `packaged_task`/`future` wiring — rather than relying on a black-box abstraction.
- **Why check `sequential matches == parallel matches`:** this is the correctness check for the parallel path. A fast but wrong answer (e.g. from a race condition or dropped task) is worse than a slow correct one — this comparison catches that class of bug.
- **Why by-value lambda capture (`[this, file, keyword]`) in the parallel path:** `file` is a loop variable; capturing by reference would have every submitted task see the same (final) value once the loop ends, since tasks run asynchronously after the loop has already moved on. By-value capture freezes each task's arguments independently.
- **Why exclude binary files:** searching binary content with a text keyword produces meaningless matches and wastes work; filtering by extension is a lightweight fix. A more robust version would sniff the first bytes of a file for null characters, similar to how `grep` decides a file is binary.

## Known limitations / possible extensions

- Extension-based binary detection is a heuristic, not exhaustive — content-based sniffing would be more robust
- No regex support yet — currently plain substring search only
- Large files are read into memory line-by-line but not memory-mapped; `mmap`-based reading would help for very large files
- No CPU affinity or NUMA-aware thread placement — relevant at larger scale, not implemented here
- Load balancing is naive (one task per file) — files of very different sizes create uneven work distribution across threads; a work-stealing queue would address this

## What I'd test next with more time

- Run under ThreadSanitizer (`-fsanitize=thread`) to verify no data races
- Profile with `perf stat` to look at cache-miss and context-switch behavior directly
- Benchmark across a wider range of thread counts to find the point where adding threads stops helping (contention/overhead crossover)