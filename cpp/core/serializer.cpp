#include "serializer.hpp"
#include <string>
#include <string_view>
#include <vector>

namespace logfire {

static void append_escaped(std::string& out, std::string_view s) {
    for (char c : s) {
        if      (c == '"')  { out += '\\'; out += '"';  }
        else if (c == '\\') { out += '\\'; out += '\\'; }
        else if (c == '\r') { out += '\\'; out += 'r';  }
        else                  out += c;
    }
}

std::string to_json(const std::vector<std::string_view>& lines) {
    if (lines.empty()) return "[]";

    std::size_t reserve = 2;
    for (auto& l : lines) reserve += l.size() + 3;

    std::string j;
    j.reserve(reserve);
    j = "[";
    for (std::size_t i = 0; i < lines.size(); ++i) {
        if (i) j += ',';
        j += '"';
        append_escaped(j, lines[i]);
        j += '"';
    }
    j += ']';
    return j;
}

} // namespace logfire
