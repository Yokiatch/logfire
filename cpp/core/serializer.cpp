#include "serializer.hpp"
#include <string>
#include <string_view>
#include <vector>

namespace logfire {

static std::string escape_json(std::string_view s) {
    std::string out;
    out.reserve(s.size() + 8);
    for (char c : s) {
        if      (c == '"')  out += "\\\"";
        else if (c == '\\') out += "\\\\";
        else if (c == '\r') out += "\\r";
        else                out += c;
    }
    return out;
}

std::string to_json(const std::vector<std::string_view>& lines) {
    std::string j = "[";
    for (std::size_t i = 0; i < lines.size(); ++i) {
        if (i) j += ',';
        j += '"';
        j += escape_json(lines[i]);
        j += '"';
    }
    j += ']';
    return j;
}

} // namespace logfire