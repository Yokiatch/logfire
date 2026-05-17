#include "line_scanner.hpp"
#include <cstring>
#include<vector>
#include<string>
namespace logfire {

std::vector<std::string_view> scan_lines(std::string_view buf) {
    std::vector<std::string_view> lines;
    lines.reserve(buf.size() / 80);

    const char* start = buf.data();
    const char* end   = start + buf.size();
    const char* cur   = start;

    while (cur < end) {
        const char* nl = static_cast<const char*>(memchr(cur, '\n', end - cur));
        if (!nl) nl = end;
        if (nl > cur) lines.emplace_back(cur, nl - cur);
        cur = nl + 1;
    }
    return lines;
}

} // namespace logfire
