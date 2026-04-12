#pragma once

#include <string>
#include <string_view>

auto Trim(const std::string& str) -> std::string;

auto ValueHasWildcard(std::string_view val) -> bool;
