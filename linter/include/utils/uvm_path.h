#pragma once

#include <filesystem>
#include <optional>
#include <string_view>

namespace uvm {

auto ResolveUvmPath() -> std::optional<std::filesystem::path>;
auto IsUvmFile(std::string_view pathStr, const std::filesystem::path& uvmDir)
    -> bool;

}  // namespace uvm
