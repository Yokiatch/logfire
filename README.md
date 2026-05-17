# logfire

High-performance log analytics engine with a C++ core and Python API.
Scans and filters millions of log lines in milliseconds using memory-mapped I/O, SIMD line scanning, and RE2 regex.

## Benchmark

| File size | Lines | Pattern | Time | Throughput |
|---|---|---|---|---|
| ~5 MB | 100,000 | `ERROR` | ~13ms | **268 MB/s** |

~15-30x faster than equivalent Python grep.

## Architecture

HTTP client
↓
FastAPI (Python)          — routing, validation, schema
↓
pybind11 bridge           — GIL release, zero-copy handoff
↓
C++ core engine
├── mmap_reader       — zero-copy memory-mapped file I/O
├── line_scanner      — SIMD-accelerated newline search (memchr/SSE2)
├── query_filter      — RE2 regex + field filtering, linear time guaranteed
└── serializer        — JSON output

## Stack

- **C++20** — core engine
- **RE2** — safe linear-time regex (no catastrophic backtracking)
- **pybind11** — Python/C++ bridge with GIL management  
- **FastAPI** — async HTTP API
- **Google Test** — C++ unit tests (13/13 passing)
- **mmap + MADV_SEQUENTIAL** — zero-copy file reading

## API

### `POST /query/`

```json
{
  "path": "/var/log/app.log",
  "pattern": "ERROR|WARN",
  "field_filter": "timeout",
  "limit": 100,
  "offset": 0
}
```

Response:
```json
{
  "count": 3,
  "lines": [
    "2024-01-01T12:00:00 ERROR connection timeout",
    "2024-01-01T13:00:00 WARN  read timeout"
  ]
}
```

### `GET /health`

```json
{"status": "ok"}
```

## Quick start

```bash
# 1. Install dependencies (Ubuntu/WSL2)
sudo apt install -y build-essential cmake ninja-build libre2-dev libgtest-dev python3-fastapi python3-uvicorn python3-pybind11 pybind11-dev

# 2. Build C++ core + pybind11 extension
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel

# 3. Run tests
cd build && ctest --output-on-failure && cd ..

# 4. Start API
PYTHONPATH=. python3 -m uvicorn python.api.main:app --port 8080

# 5. Query
curl -X POST http://localhost:8080/query/ \
  -H "Content-Type: application/json" \
  -d '{"path":"/var/log/syslog","pattern":"error","limit":50}'
```

## Project structure