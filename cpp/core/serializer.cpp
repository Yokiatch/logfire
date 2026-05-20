#include "serializer.hpp"
#include <string>
#include<cstring>
#include <string_view>
#include <vector>

namespace logfire {

static void append_escaped(std::string& out, std::string_view s) {
    for (char c : s) {
        if      (c == '"')  { out += '\\'; out += '"';  }
        else if (c == '\\') { out += '\\'; out += '\\'; }
        else if (c == '\r') { out += '\\'; out += 'r';  }
        else                  out.push_back(c);
    }
}

static inline bool needs_escape(std::string_view s) {
    return memchr(s.data(), '"', s.size()) ||
           memchr(s.data(), '\\', s.size()) ||
           memchr(s.data(), '\r', s.size());
}

std::string to_json(const std::vector<std::string_view>& lines) {
    if (lines.empty()) return "[]";

    std::size_t reserve = 2;
    for (auto& l : lines) reserve += l.size() + 3;

    std::string j;
    j.reserve(reserve);
    j.push_back('[');
    for (std::size_t i = 0; i < lines.size(); ++i) {
        if (i) j.push_back(',');
        j.push_back('"');
        if (needs_escape(lines[i]))
            append_escaped(j, lines[i]);
        else
            j.append(lines[i]);
        j.push_back('"');
    }
    j.push_back(']');
    return j;
}

} // namespace logfire
