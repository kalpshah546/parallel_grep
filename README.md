# Parallel Grep — Multithreaded C++ File Search & Performance Analytics

A high-performance command-line search engine built in C++ that scans directory trees concurrently using a custom-built thread pool. 

To go beyond standard execution, this repository includes an end-to-end Python benchmarking and data analysis suite (`benchmark.py` & `analyze_benchmarks.py`) that measures runtime scalability, throughput limits, and hardware resource saturation to deliver data-backed deployment recommendations.

---

## 💡 Why This Project?

When searching through thousands of log files or large codebases, single-threaded tools spend most of their time waiting on disk I/O instead of utilizing modern multi-core processors. 

This project solves that by distributing file reading across worker threads. Rather than using higher-level abstractions like `std::async`, the core thread pool, task queue, and worker synchronization are implemented completely **from scratch in C++** for full visibility and control over thread lifecycles.

---

## ✨ Features

- **Custom C++ Thread Pool**: Built with worker thread loops, generic task queues, and `std::future` result handling.
- **Concurrent Directory Search**: Recursively traverses file paths and scans file contents in parallel.
- **Smart Filtering**: Automatically filters out binary file formats to prevent unreadable search noise.
- **Cancellation Awareness**: Supports early termination signals to stop work in flight.
- **Automated Benchmarking Harness (`benchmark.py`)**: Runs parameter sweeps across file sizes (1MB to 500MB), worker counts (1 to 32), and pattern complexities.
- **Data Analysis & Visualization (`analyze_benchmarks.py`)**: Uses Pandas and Matplotlib to aggregate performance metrics, detect hardware bottlenecks, and plot performance charts.

---

## 🛠️ Project Structure

```text
├── main.cpp                # CLI entry point, flag parsing & search dispatcher
├── threadpool.h / .cpp     # Thread pool with synchronized worker queue
├── searcher.h / .cpp       # Recursive file search logic (sequential & parallel)
├── benchmark.h / .cpp      # C++ execution timing & speedup metrics
├── benchmark.py            # Python automated benchmark suite across configuration matrix
├── analyze_benchmarks.py   # Pandas data analysis & Matplotlib chart generator
├── requirements.txt        # Python dependencies (pandas, matplotlib, psutil)
├── benchmark_results.csv   # Raw benchmark metric data
├── benchmark_summary.csv   # Aggregated performance summary table
└── charts/                 # Generated performance visualization plots
```

---

## 🚀 Quickstart & Usage

### 1. Build the C++ Executable
Requires a C++17 compatible compiler and CMake:

```bash
cmake -S . -B build
cmake --build build
```
*(Executable is generated at `./build/parallel_grep.exe`)*

### 2. Run a Search
```bash
./build/parallel_grep <directory> <keyword> [thread_count]
```

Example:
```bash
./build/parallel_grep ./test_data ERROR 4
```

Output:
```text
Searching './test_data' for "ERROR" using 4 threads...

========== Results ==========
Files processed: 2000
Matches: 666
Workers: 4
Time: 362.3 ms
Speedup: 82.4x
==============================
```

---

## 📊 Performance Analysis & Data Insights

### Business & Technical Framing
To evaluate real-world ROI when scaling compute resources, we benchmarked the tool across a matrix of 48 distinct execution environments. The goal was to identify the exact **point of diminishing returns** where adding more CPU threads stops improving speed and starts wasting resources.

### Benchmark Matrix Methodology
- **Dataset Sizes**: 1 MB, 10 MB, 100 MB, and 500 MB synthetic log file directories.
- **Worker Counts**: 1, 2, 4, 8, 16, and 32 threads.
- **Search Complexity**: Simple literal keyword (`ERROR`) vs. complex multi-token log signatures.
- **Metrics Tracked**: Wall-clock runtime (s), processing throughput (MB/s), and system CPU utilization (%).

---

### 🔑 Key Findings (Empirical Data)

1. **Optimal Processing Scale (Peak Throughput)**:
   - Maximum processing speed peaked at **85.07 MB/s** on 100 MB datasets using **4 worker threads**.
2. **Diminishing Returns Threshold**:
   - Scaling beyond **4 worker threads** flattens processing throughput across all dataset sizes. On 500 MB workloads, increasing threads from 4 to 32 increased thread contention overhead without improving wall-clock runtime.
3. **Hardware Bottleneck Transition**:
   - Up to 4 threads, performance is **CPU-bound** (CPU utilization scales naturally up to ~197%). Beyond 4 threads, disk I/O bandwidth becomes the main bottleneck, preventing further speed gains regardless of core count.

---

### 📈 Performance Visualizations

#### 1. Processing Throughput vs. Thread Count
![Throughput vs Thread Count](charts/throughput_vs_threads.png)
*Throughput scales rapidly up to 4 threads, reaching ~85 MB/s before flattening due to I/O constraints.*

#### 2. System CPU Utilization Scaling
![CPU Utilization vs Thread Count](charts/cpu_vs_threads.png)
*CPU usage increases proportionally with workers up to the physical/logical core limit.*

#### 3. Execution Runtime vs. File Dataset Size
![Runtime vs File Size](charts/runtime_vs_filesize.png)
*Linear scalability of wall-clock execution time across increasing dataset sizes at 4 worker threads.*

---

## 💡 Executive Recommendation

> **Actionable Takeaway for System Resource Allocation:**
> - **Small Workloads (< 50 MB)**: Allocate **2 worker threads**. This minimizes thread creation overhead while doubling search speed over single-threaded execution.
> - **Large Workloads (100 MB – 500+ MB)**: Cap thread allocation at **4 worker threads**. This achieves peak processing throughput (**~85 MB/s**) while maintaining optimal CPU-to-disk I/O efficiency without thrashing hardware resource queues.

---

## 🧪 Running the Analysis Harness

To run the benchmarking suite and regenerate analysis charts yourself:

```bash
# 1. Install dependencies
pip install -r requirements.txt

# 2. Run benchmark matrix sweep
python benchmark.py

# 3. Generate analysis reports and charts
python analyze_benchmarks.py
```
