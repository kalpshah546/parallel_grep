import os
from pathlib import Path
import pandas as pd
import matplotlib.pyplot as plt

# ==============================================================================
# Business Analyst Performance Benchmark Analysis Script
# 
# Purpose:
# Analyzes runtime, throughput (MB/s), and CPU utilization across thread counts
# and file sizes to identify optimal parallel execution configurations and bottlenecks.
# ==============================================================================

# Define input/output file paths relative to script root
BASE_DIR = Path(__file__).parent.resolve()
RESULTS_CSV = BASE_DIR / "benchmark_results.csv"
SUMMARY_CSV = BASE_DIR / "benchmark_summary.csv"
CHARTS_DIR = BASE_DIR / "charts"

CHARTS_DIR.mkdir(parents=True, exist_ok=True)


def analyze_performance():
    # Plain-English Explanation:
    # 1. Load raw benchmark data collected by benchmark.py into a Pandas DataFrame.
    if not RESULTS_CSV.exists():
        raise FileNotFoundError(f"Input file '{RESULTS_CSV}' not found. Run benchmark.py first.")

    df = pd.read_csv(RESULTS_CSV)
    print(f"Loaded {len(df)} benchmark records from '{RESULTS_CSV}'.")

    # Plain-English Explanation:
    # 2. Compute Throughput vs. Thread Count to spot the point of diminishing returns.
    # We group by dataset size and thread count, computing the average throughput (MB/s).
    throughput_summary = df.groupby(["file_size_tier_mb", "thread_count"])["throughput_mbs"].mean().unstack(level=0)
    
    # Plain-English Explanation:
    # Calculate point of diminishing returns per file size tier:
    # Defined as the lowest thread count that achieves at least 90% of the maximum throughput for that tier.
    diminishing_returns = {}
    for size in df["file_size_tier_mb"].unique():
        tier_data = df[df["file_size_tier_mb"] == size].groupby("thread_count")["throughput_mbs"].mean()
        max_throughput = tier_data.max()
        # Filter thread counts reaching >= 90% of max throughput
        near_optimal = tier_data[tier_data >= 0.90 * max_throughput]
        diminishing_returns[size] = int(near_optimal.index.min())

    print("\n--- Point of Diminishing Returns by File Size Tier ---")
    for size in sorted(diminishing_returns.keys()):
        print(f"  • {size:3d}MB Tier: Diminishing returns occur beyond {diminishing_returns[size]} threads")

    # Plain-English Explanation:
    # 3. Identify CPU utilization vs Thread Count to determine when CPU vs I/O bounds the system.
    cpu_summary = df.groupby(["file_size_tier_mb", "thread_count"])["cpu_utilization_pct"].mean().unstack(level=0)

    # Plain-English Explanation:
    # 4. Extract overall best-performing configuration and best configuration per tier.
    best_overall_row = df.loc[df["throughput_mbs"].idxmax()]
    
    summary_rows = []
    for size in sorted(df["file_size_tier_mb"].unique()):
        tier_df = df[df["file_size_tier_mb"] == size]
        best_tier_row = tier_df.loc[tier_df["throughput_mbs"].idxmax()]
        opt_threads = diminishing_returns[size]
        
        summary_rows.append({
            "file_size_tier_mb": size,
            "optimal_thread_count": opt_threads,
            "max_throughput_mbs": round(best_tier_row["throughput_mbs"], 2),
            "min_runtime_sec": round(best_tier_row["runtime_sec"], 4),
            "peak_cpu_utilization_pct": round(tier_df["cpu_utilization_pct"].max(), 2),
            "best_pattern_complexity": best_tier_row["pattern_complexity"]
        })

    summary_df = pd.DataFrame(summary_rows)
    summary_df.to_csv(SUMMARY_CSV, index=False)
    print(f"\nSaved benchmark summary report to '{SUMMARY_CSV}'.")

    # ==========================================================================
    # Matplotlib Visualization Generation
    # ==========================================================================
    plt.style.use('seaborn-v0_8-whitegrid' if 'seaborn-v0_8-whitegrid' in plt.style.available else 'default')
    palette = ['#1f77b4', '#2ca02c', '#ff7f0e', '#d62728']

    # Chart 1: Throughput vs. Thread Count (Line Chart per dataset size)
    plt.figure(figsize=(9, 5.5))
    sizes = sorted(df["file_size_tier_mb"].unique())
    for idx, size in enumerate(sizes):
        subset = df[df["file_size_tier_mb"] == size].groupby("thread_count")["throughput_mbs"].mean()
        plt.plot(subset.index, subset.values, marker='o', linewidth=2.2, color=palette[idx % len(palette)], label=f"{size} MB Dataset")

    plt.title("Parallel Grep: Throughput vs. Thread Count", fontsize=13, fontweight='bold', pad=12)
    plt.xlabel("Worker Thread Count", fontsize=11)
    plt.ylabel("Processing Throughput (MB/s)", fontsize=11)
    plt.xticks([1, 2, 4, 8, 16, 32])
    plt.legend(title="Dataset Size", loc='upper left', frameon=True)
    plt.grid(True, linestyle='--', alpha=0.5)
    plt.tight_layout()
    plt.savefig(CHARTS_DIR / "throughput_vs_threads.png", dpi=300)
    plt.close()

    # Chart 2: CPU Utilization vs. Thread Count
    plt.figure(figsize=(9, 5.5))
    for idx, size in enumerate(sizes):
        subset = df[df["file_size_tier_mb"] == size].groupby("thread_count")["cpu_utilization_pct"].mean()
        plt.plot(subset.index, subset.values, marker='s', linestyle='--', linewidth=2.0, color=palette[idx % len(palette)], label=f"{size} MB Dataset")

    plt.title("Parallel Grep: CPU Utilization vs. Thread Count", fontsize=13, fontweight='bold', pad=12)
    plt.xlabel("Worker Thread Count", fontsize=11)
    plt.ylabel("CPU Utilization (%)", fontsize=11)
    plt.xticks([1, 2, 4, 8, 16, 32])
    plt.legend(title="Dataset Size", loc='upper left', frameon=True)
    plt.grid(True, linestyle='--', alpha=0.5)
    plt.tight_layout()
    plt.savefig(CHARTS_DIR / "cpu_vs_threads.png", dpi=300)
    plt.close()

    # Chart 3: Runtime vs. File Size at Optimal Thread Count (e.g., 4 threads)
    plt.figure(figsize=(9, 5.5))
    opt_threads_selected = 4
    runtime_by_size = df[df["thread_count"] == opt_threads_selected].groupby("file_size_tier_mb")["runtime_sec"].mean()

    plt.plot(runtime_by_size.index, runtime_by_size.values, marker='^', linewidth=2.4, color='#9467bd', label=f"Runtime at {opt_threads_selected} Worker Threads")
    plt.title(f"Parallel Grep: Runtime vs. File Size ({opt_threads_selected} Threads)", fontsize=13, fontweight='bold', pad=12)
    plt.xlabel("Dataset File Size Tier (MB)", fontsize=11)
    plt.ylabel("Wall-Clock Runtime (Seconds)", fontsize=11)
    plt.legend(loc='upper left', frameon=True)
    plt.grid(True, linestyle='--', alpha=0.5)
    plt.tight_layout()
    plt.savefig(CHARTS_DIR / "runtime_vs_filesize.png", dpi=300)
    plt.close()

    print(f"Successfully rendered performance charts to '{CHARTS_DIR}'.")


if __name__ == "__main__":
    analyze_performance()
