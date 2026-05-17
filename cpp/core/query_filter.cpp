#include "query_filter.hpp"
#include <re2/re2.h>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace logfire {

QueryFilter::QueryFilter(const QueryOptions& opts) : opts_(opts) {
    if (!opts.pattern.empty()) {
        RE2::Options re_opts;
        re_opts.set_case_sensitive(false);
        re_ = std::make_unique<re2::RE2>(opts.pattern, re_opts);
        if (!re_->ok())
            throw std::invalid_argument("Bad regex: " + re_->error());
    }
}

QueryFilter::~QueryFilter() = default;

std::vector<std::string_view>
QueryFilter::apply(const std::vector<std::string_view>& lines) const {
    std::vector<std::string_view> out;
    out.reserve(lines.size());

    std::size_t skipped = 0;
    for (auto& line : lines) {
        if (re_ && !RE2::PartialMatch({line.data(), line.size()}, *re_)) continue;
        if (!opts_.field_filter.empty() &&
            line.find(opts_.field_filter) == std::string_view::npos) continue;
        if (skipped++ < opts_.offset) continue;
        out.push_back(line);
        if (opts_.limit && out.size() >= opts_.limit) break;
    }
    return out;
}

} // namespace logfire