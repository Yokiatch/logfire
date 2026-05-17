#pragma once
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace re2 { class RE2; }

namespace logfire {

struct QueryOptions {
    std::string pattern;
    std::string field_filter;
    std::size_t limit  = 0;
    std::size_t offset = 0;
};

class QueryFilter {
public:
    explicit QueryFilter(const QueryOptions& opts);
    ~QueryFilter();

    std::vector<std::string_view>
    apply(const std::vector<std::string_view>& lines) const;

private:
    QueryOptions              opts_;
    std::unique_ptr<re2::RE2> re_;
};

} // namespace logfire