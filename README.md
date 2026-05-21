# Logfire --- High-Performance Log Analytics Engine

Logfire is a C++ log analytics/query engine designed for fast searching
over large log files using memory-mapped I/O, single-pass filtering, and
profiling-driven optimization.

## Highlights

-   Memory-mapped file access (`mmap`)
-   Literal + regex search (RE2)
-   Single-pass scan + filter pipeline
-   Python bindings via pybind11
-   CLI benchmarking support
-   Profiling with `perf`
-   Benchmark dashboards comparing:
    -   Logfire C++ engine
    -   Python regex
    -   grep
    -   ripgrep

## Performance Snapshot

Recent optimized benchmarks on:

-   Ryzen 5 5600H
-   WSL2 Ubuntu
-   GCC 15 (`-O3`, `-march=native`)
-   Warm cache
-   \~44 MB log file (\~1M lines)

Observed:

-   Median query latency: \~93 ms
-   Throughput: \~476 MB/s
-   \~32% lower latency after profiling + serializer optimizations

Optimization journey:

136 ms → 113 ms → 98 ms → 93 ms

## Architecture

``` text
mmap
↓
apply_single_pass()
↓
QueryFilter::matches()
↓
JSON serialization
↓
CLI / Python API
```

Core files:

``` text
cpp/core/
├── mmap_reader.cpp
├── line_scanner.cpp
├── query_filter.cpp
├── serializer.cpp
├── py_bindings.cpp
└── cli/main.cpp
```

## Build

Requirements:

-   GCC ≥ 15
-   CMake ≥ 4
-   Python (optional)
-   RE2
-   pybind11

Build:

``` bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

Run CLI:

``` bash
./build/logfire_cli ~/bench.log --pattern ERROR --bench
```

## Benchmarking

Use repeated runs and medians:

``` bash
for i in {1..10}; do
taskset -c 3 ./build/logfire_cli ~/bench.log \
--pattern ERROR --bench
done
```

Profile:

``` bash
perf record -g taskset -c 3 ./build/logfire_cli ~/bench.log \
--pattern ERROR --bench

perf report
```

## Lessons from Optimization

Several attempted optimizations were rejected after benchmarking because
they made performance worse. Improvements are accepted only when median
measurements improve.

Workflow:

baseline → modify → benchmark → compare → keep/revert

## Roadmap

-   Advanced queries
-   Aggregations
-   Parallel scan experiments
-   Time filters
-   Structured log extraction

## License

GNU
