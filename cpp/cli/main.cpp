#include "mmap_reader.hpp"
#include "line_scanner.hpp"
#include "query_filter.hpp"
#include "serializer.hpp"
#include <iostream>
#include <string>
#include <chrono>
#include <cstdlib>

static void usage(const char* prog) {
    std::cerr << "Usage: " << prog
              << " <file> [--pattern <regex>] [--field <str>]"
                 " [--limit <n>] [--offset <n>] [--bench]\n";
    std::exit(1);
}

int main(int argc, char* argv[]) {
    if (argc < 2) usage(argv[0]);

    std::string path;
    logfire::QueryOptions opts;
    bool bench_mode = false;

    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if      (a == "--pattern" && i+1 < argc) opts.pattern      = argv[++i];
        else if (a == "--field"   && i+1 < argc) opts.field_filter = argv[++i];
        else if (a == "--limit"   && i+1 < argc) opts.limit        = std::stoul(argv[++i]);
        else if (a == "--offset"  && i+1 < argc) opts.offset       = std::stoul(argv[++i]);
        else if (a == "--bench")                  bench_mode        = true;
        else if (a[0] != '-')                     path              = a;
        else usage(argv[0]);
    }

    if (path.empty()) usage(argv[0]);

    try {
        auto t0 = std::chrono::high_resolution_clock::now();

        auto file    = logfire::MmapFile::open(path);
        auto lines   = logfire::scan_lines(file.view());
        logfire::QueryFilter filter{opts};
        auto matched = filter.apply(lines);

        auto t1 = std::chrono::high_resolution_clock::now();
        double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

        if (bench_mode) {
            std::cout << "total="         << lines.size()
                      << " matched="      << matched.size()
                      << " time_ms="      << ms
                      << " throughput_MB/s="
                      << (file.size / 1e6) / (ms / 1e3)
                      << "\n";
        } else {
            for (auto& l : matched)
                std::cout << l << "\n";
            std::cerr << "── " << matched.size() << "/" << lines.size()
                      << " lines matched in " << ms << " ms\n";
        }

        file.close();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}