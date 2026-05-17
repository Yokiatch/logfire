#pragma once
#include <string>
#include <string_view>
#include <vector>

namespace logfire {

std::string to_json(const std::vector<std::string_view>& lines);

} // namespace logfire