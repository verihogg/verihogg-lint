#include "utils/string_utils.h"

#include <algorithm>
#include <string>
#include <string_view>

auto Trim(const std::string& str) -> std::string {
  auto start = str.find_first_not_of(" \t\n\r");
  if (start == std::string::npos) {
    return "";
  }
  auto end = str.find_last_not_of(" \t\n\r");
  return str.substr(start, end - start + 1);
}

auto ValueHasWildcard(std::string_view val) -> bool {
  static constexpr std::string_view kWildcardChars = "xXzZ?";
  return std::ranges::any_of(val, [](char chr) -> bool {
    return kWildcardChars.find(chr) != std::string_view::npos;
  });
}
