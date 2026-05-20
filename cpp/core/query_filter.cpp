#include "query_filter.hpp"
#include <re2/re2.h>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
#include <cstring>

namespace logfire {

static bool detect_literal(const std::string& p) {
    return p.find_first_of("^$.*+?[]{}()|\\") == std::string::npos;
}

QueryFilter::QueryFilter(const QueryOptions& opts) : opts_(opts) {
    if (!opts.pattern.empty()) {
        is_literal_ = detect_literal(opts.pattern);
        if (!is_literal_) {
            RE2::Options re_opts;
            re_opts.set_case_sensitive(false);
            re_ = std::make_unique<re2::RE2>(opts.pattern, re_opts);
            if (!re_->ok())
                throw std::invalid_argument("Bad regex: " + re_->error());
        }
    }
}

QueryFilter::~QueryFilter() = default;

// Shared match check — used by both apply() and apply_single_pass()
bool QueryFilter::matches(std::string_view line) const {
    if (!opts_.pattern.empty()) {
        if (is_literal_) {
            if (line.find(opts_.pattern) == std::string_view::npos)
                return false;
        } else {
            if (!RE2::PartialMatch({line.data(), line.size()}, *re_))
                return false;
        }
    }
    if (!opts_.field_filter.empty() &&
        line.find(opts_.field_filter) == std::string_view::npos)
        return false;
    return true;
}

// Two-pass: operates on pre-scanned line views
std::vector<std::string_view>
QueryFilter::apply(const std::vector<std::string_view>& lines) const {
    std::vector<std::string_view> out;
    out.reserve(lines.size() / 4);

    std::size_t skipped = 0;
    for (auto& line : lines) {
        if (!matches(line)) continue;
        if (skipped++ < opts_.offset) continue;
        out.push_back(line);
        if (opts_.limit && out.size() >= opts_.limit) break;
    }
    return out;
}

// Single-pass: scan newlines + filter in one loop
// Never builds the full vector<string_view> of all lines
std::vector<std::string_view>
QueryFilter::apply_single_pass(std::string_view buf) const {
    std::vector<std::string_view> out;
    out.reserve(65536);  // start with 64k slots, grows as needed

    const char* start = buf.data();
    const char* end   = start + buf.size();
    const char* cur   = start;

    std::size_t skipped = 0;

    while (cur < end) {
        // find next newline — memchr compiles to AVX2 on -march=native
        const char* nl = static_cast<const char*>(
            memchr(cur, '\n', end - cur));
        if (!nl) nl = end;

        if (nl > cur) {
            std::string_view line{cur, static_cast<std::size_t>(nl - cur)};

            if (matches(line)) {
                if (skipped++ >= opts_.offset) {
                    out.push_back(line);
                    if (opts_.limit && out.size() >= opts_.limit) break;
                }
            }
        }
        cur = nl + 1;
    }
    return out;
}

} // namespace logfire
