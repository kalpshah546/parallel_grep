import os
import sys
import time
import subprocess
import csv
from pathlib import Path

try:
    import psutil
except ImportError:
    psutil = None


BASE_DIR = Path(__file__).parent.resolve()
EXE_PATH_BUILD = BASE_DIR / "build" / "parallel_grep.exe"
EXE_PATH_ROOT = BASE_DIR / "parallel_grep.exe"
SYNTHETIC_DIR = BASE_DIR / "synthetic_test_data"
RESULTS_CSV = BASE_DIR / "benchmark_results.csv"


def find_or_build_executable():
    """Locate the parallel_grep executable or trigger cmake build if missing."""
    if EXE_PATH_BUILD.exists():
        return str(EXE_PATH_BUILD.resolve())
    if EXE_PATH_ROOT.exists():
        return str(EXE_PATH_ROOT.resolve())
    
    print("Executable not found. Attempting build via CMake...")
    os.makedirs("build", exist_ok=True)
    subprocess.run(["cmake", "-S", ".", "-B", "build"], check=True)
    subprocess.run(["cmake", "--build", "build"], check=True)
    
    if EXE_PATH_BUILD.exists():
        return str(EXE_PATH_BUILD.resolve())
    raise FileNotFoundError("Could not find or build parallel_grep executable.")


def generate_synthetic_dataset(target_dir: Path, total_target_mb: float, file_count: int = 50):
    """Generate synthetic log files totaling target_target_mb."""
    target_dir.mkdir(parents=True, exist_ok=True)
    
    # Calculate bytes per file
    total_bytes = int(total_target_mb * 1024 * 1024)
    bytes_per_file = total_bytes // file_count
    
    lines_template = [
        "2026-08-27 10:00:01 INFO [MainThread] System initialized successfully.\n",
        "2026-08-27 10:00:02 DEBUG [Worker-1] Processing batch request payload.\n",
        "2026-08-27 10:00:03 ERROR [Worker-4] Database connection timeout encountered.\n",
        "2026-08-27 10:00:04 WARN [Worker-2] Memory utilization reached 85% threshold.\n",
        "2026-08-27 10:00:05 CRITICAL_SYSTEM_FAILURE_LOG_ENTRY_HEX_0x4F8A Core dumped.\n",
        "2026-08-27 10:00:06 INFO [Worker-3] Rescheduling worker task queue items.\n"
    ]
    
    block_str = "".join(lines_template)
    block_bytes = len(block_str.encode('utf-8'))
    blocks_per_file = max(1, bytes_per_file // block_bytes)
    
    actual_bytes = 0
    for i in range(file_count):
        file_path = target_dir / f"log_sample_{i+1:04d}.log"
        if not file_path.exists() or file_path.stat().st_size < bytes_per_file:
            with open(file_path, "w", encoding="utf-8") as f:
                for _ in range(blocks_per_file):
                    f.write(block_str)
        actual_bytes += file_path.stat().st_size
        
    return actual_bytes / (1024 * 1024)


def measure_cpu_utilization(proc, interval=0.01):
    """Utility to poll CPU usage of process and children if psutil available."""
    if not psutil:
        return 0.0
    try:
        p = psutil.Process(proc.pid)
        cpu_samples = []
        while proc.poll() is None:
            try:
                cpu_samples.append(p.cpu_percent(interval=interval))
            except (psutil.NoSuchProcess, psutil.AccessDenied):
                break
        return sum(cpu_samples) / len(cpu_samples) if cpu_samples else 0.0
    except Exception:
        return 0.0


def run_benchmark_matrix():
    exe = find_or_build_executable()
    print(f"Using executable: {exe}")
    
    # Define test parameters
    sizes_mb = [1, 10, 100, 500]
    threads = [1, 2, 4, 8, 16, 32]
    patterns = [
        ("simple", "ERROR"),
        ("complex", "CRITICAL_SYSTEM_FAILURE_LOG_ENTRY_HEX_0x4F8A")
    ]
    
    results = []
    
    print("\n--- Starting Benchmark Matrix ---")
    for size_mb in sizes_mb:
        dir_name = SYNTHETIC_DIR / f"data_{size_mb}MB"
        print(f"\n[Dataset] Preparing {size_mb}MB test files in '{dir_name}'...")
        file_count = 10 if size_mb <= 10 else (50 if size_mb <= 100 else 100)
        actual_mb = generate_synthetic_dataset(dir_name, size_mb, file_count=file_count)
        
        for pattern_name, keyword in patterns:
            for thread_count in threads:
                cmd = [exe, str(dir_name), keyword, str(thread_count)]
                
                if psutil:
                    psutil.cpu_percent(interval=None)
                
                start_time = time.perf_counter()
                proc = subprocess.run(cmd, capture_output=True, text=True)
                end_time = time.perf_counter()
                
                if psutil:
                    cpu_pct = psutil.cpu_percent(interval=None)
                else:
                    cpu_pct = 0.0
                    
                stdout, stderr = proc.stdout, proc.stderr
                runtime_sec = max(end_time - start_time, 0.0001)
                throughput_mbs = actual_mb / runtime_sec
                
                # Extract processed stats from C++ stdout if present
                files_processed = file_count
                matches_found = 0
                for line in stdout.splitlines():
                    if "Files processed:" in line:
                        try:
                            files_processed = int(line.split(":")[1].strip())
                        except Exception:
                            pass
                    elif "Matches:" in line:
                        try:
                            matches_found = int(line.split(":")[1].strip())
                        except Exception:
                            pass

                row = {
                    "file_size_tier_mb": size_mb,
                    "actual_size_mb": round(actual_mb, 2),
                    "thread_count": thread_count,
                    "pattern_complexity": pattern_name,
                    "keyword": keyword,
                    "runtime_sec": round(runtime_sec, 4),
                    "throughput_mbs": round(throughput_mbs, 2),
                    "cpu_utilization_pct": round(cpu_pct, 2),
                    "files_processed": files_processed,
                    "matches_found": matches_found
                }
                results.append(row)
                
                print(f"Size: {size_mb}MB | Threads: {thread_count:2d} | Pattern: {pattern_name:7s} | "
                      f"Time: {runtime_sec:.4f}s | Speed: {throughput_mbs:.2f} MB/s | CPU: {cpu_pct:.1f}%")

    # Write to CSV
    fieldnames = [
        "file_size_tier_mb", "actual_size_mb", "thread_count",
        "pattern_complexity", "keyword", "runtime_sec",
        "throughput_mbs", "cpu_utilization_pct", "files_processed", "matches_found"
    ]
    
    with open(RESULTS_CSV, "w", newline="", encoding="utf-8") as f:
        writer = csv.DictWriter(f, fieldnames=fieldnames)
        writer.writeheader()
        writer.writerows(results)
        
    print(f"\nBenchmark completed successfully. Saved {len(results)} rows to '{RESULTS_CSV}'.")


if __name__ == "__main__":
    run_benchmark_matrix()
