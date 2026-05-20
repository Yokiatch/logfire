#include "mmap_reader.hpp"
#include "line_scanner.hpp"
#include "query_filter.hpp"
#include "serializer.hpp"
#include <iostream>
#include <string>
#include <chrono>
#include <cstdlib>

using clk = std::chrono::high_resolution_clock;

static double ms(clk::time_point a, clk::time_point b) {
    return std::chrono::duration<double, std::milli>(b - a).count();
}

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
        // ── mmap ─────────────────────────────────────────────────
        auto t0   = clk::now();
        auto file = logfire::MmapFile::open(path);
        auto t1   = clk::now();

        // ── single-pass: scan + filter fused ─────────────────────
        logfire::QueryFilter filter{opts};
        auto matched = filter.apply_single_pass(file.view());
        auto t2      = clk::now();

        // ── serialize ─────────────────────────────────────────────
        auto result = logfire::to_json(matched);
        auto t3     = clk::now();

        double t_mmap   = ms(t0, t1);
        double t_filter = ms(t1, t2);
        double t_serial = ms(t2, t3);
        double t_total  = ms(t0, t3);
        double file_mb  = file.size / 1e6;

        if (bench_mode) {
            std::cout << "total="    << matched.size()
                      << " time_ms=" << t_total
                      << " throughput_MB/s=" << (file_mb / (t_total / 1e3))
                      << "\n";
            std::cerr << "\n── stage breakdown (single-pass) ────\n"
                      << "  mmap         : " << t_mmap   << " ms\n"
                      << "  scan+filter  : " << t_filter << " ms\n"
                      << "  serialize    : " << t_serial << " ms\n"
                      << "  total        : " << t_total  << " ms\n"
                      << "  file         : " << file_mb  << " MB\n"
                      << "─────────────────────────────────────\n";
        } else {
            for (auto& l : matched)
                std::cout << l << "\n";
            std::cerr << "── " << matched.size()
                      << " lines matched in " << t_total << " ms\n";
        }

        file.close();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
