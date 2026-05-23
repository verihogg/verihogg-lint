#pragma once

#include <filesystem>
#include <optional>

namespace uvm {

auto ResolveUvmPath() -> std::optional<std::filesystem::path>;

}  // namespace uvm
