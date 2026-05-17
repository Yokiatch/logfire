#pragma once
#include <string_view>
#include <vector>

namespace logfire {

std::vector<std::string_view> scan_lines(std::string_view buf);

} // namespace logfire