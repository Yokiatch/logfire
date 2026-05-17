#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>

#include "mmap_reader.hpp"
#include "line_scanner.hpp"
#include "query_filter.hpp"
#include "serializer.hpp"

namespace fs = std::filesystem;

// ── Fixture: writes a temp log file ──────────────────────────────
struct CoreTest : ::testing::Test {
    fs::path path;
    void SetUp() override {
        path = fs::temp_directory_path() / "logfire_test.log";
        std::ofstream f(path);
        f << "2024-01-01 INFO  service started\n"
          << "2024-01-01 ERROR disk full on /dev/sda\n"
          << "2024-01-01 WARN  memory at 90%\n"
          << "2024-01-01 ERROR connection timeout\n"
          << "2024-01-01 INFO  request processed\n";
    }
    void TearDown() override { fs::remove(path); }
};

// ── mmap_reader ───────────────────────────────────────────────────
TEST_F(CoreTest, MmapOpensAndReadsFile) {
    auto f = logfire::MmapFile::open(path.string());
    EXPECT_GT(f.size, 0u);
    EXPECT_NE(f.data, nullptr);
    f.close();
}

TEST_F(CoreTest, MmapThrowsOnMissingFile) {
    EXPECT_THROW(logfire::MmapFile::open("/no/such/file.log"), std::runtime_error);
}

// ── line_scanner ──────────────────────────────────────────────────
TEST_F(CoreTest, ScannerReturnsCorrectLineCount) {
    auto f     = logfire::MmapFile::open(path.string());
    auto lines = logfire::scan_lines(f.view());
    EXPECT_EQ(lines.size(), 5u);
    f.close();
}

TEST_F(CoreTest, ScannerHandlesEmptyBuffer) {
    auto lines = logfire::scan_lines("");
    EXPECT_TRUE(lines.empty());
}

TEST_F(CoreTest, ScannerPreservesContent) {
    auto f     = logfire::MmapFile::open(path.string());
    auto lines = logfire::scan_lines(f.view());
    EXPECT_EQ(lines[0], "2024-01-01 INFO  service started");
    EXPECT_EQ(lines[1], "2024-01-01 ERROR disk full on /dev/sda");
    f.close();
}

// ── query_filter ──────────────────────────────────────────────────
TEST_F(CoreTest, NoFilterMatchesAll) {
    auto f       = logfire::MmapFile::open(path.string());
    auto lines   = logfire::scan_lines(f.view());
    logfire::QueryFilter filter{{}};
    auto matched = filter.apply(lines);
    EXPECT_EQ(matched.size(), 5u);
    f.close();
}

TEST_F(CoreTest, RegexFilterMatchesErrors) {
    auto f       = logfire::MmapFile::open(path.string());
    auto lines   = logfire::scan_lines(f.view());
    logfire::QueryOptions opts; opts.pattern = "ERROR";
    logfire::QueryFilter filter{opts};
    auto matched = filter.apply(lines);
    EXPECT_EQ(matched.size(), 2u);
    for (auto& l : matched) EXPECT_NE(l.find("ERROR"), std::string_view::npos);
    f.close();
}

TEST_F(CoreTest, FieldFilterMatchesINFO) {
    auto f       = logfire::MmapFile::open(path.string());
    auto lines   = logfire::scan_lines(f.view());
    logfire::QueryOptions opts; opts.field_filter = "INFO";
    logfire::QueryFilter filter{opts};
    EXPECT_EQ(filter.apply(lines).size(), 2u);
    f.close();
}

TEST_F(CoreTest, LimitClampResults) {
    auto f       = logfire::MmapFile::open(path.string());
    auto lines   = logfire::scan_lines(f.view());
    logfire::QueryOptions opts; opts.limit = 2;
    logfire::QueryFilter filter{opts};
    EXPECT_EQ(filter.apply(lines).size(), 2u);
    f.close();
}

TEST_F(CoreTest, OffsetSkipsLines) {
    auto f       = logfire::MmapFile::open(path.string());
    auto lines   = logfire::scan_lines(f.view());
    logfire::QueryOptions opts; opts.offset = 3;
    logfire::QueryFilter filter{opts};
    EXPECT_EQ(filter.apply(lines).size(), 2u);
    f.close();
}

TEST_F(CoreTest, BadRegexThrows) {
    logfire::QueryOptions opts; opts.pattern = "[invalid";
    EXPECT_THROW(logfire::QueryFilter{opts}, std::invalid_argument);
}

// ── serializer ────────────────────────────────────────────────────
TEST(SerializerTest, EmptyProducesEmptyArray) {
    EXPECT_EQ(logfire::to_json({}), "[]");
}

TEST(SerializerTest, EscapesQuotes) {
    std::string s = R"(say "hello")";
    std::vector<std::string_view> v{s};
    auto j = logfire::to_json(v);
    EXPECT_NE(j.find(R"(\")"), std::string::npos);
}