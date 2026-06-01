#include "utils/uvm_path.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>

namespace uvm {

namespace fs = std::filesystem;

namespace {
auto GetEnvVar(std::string_view name) -> std::optional<std::string> {
  std::ifstream envFile("/proc/self/environ", std::ios::binary);
  std::string entry;
  while (std::getline(envFile, entry, '\0')) {
    if (entry.starts_with(name) && entry.size() > name.size() &&
        entry.at(name.size()) == '=') {
      return entry.substr(name.size() + 1);
    }
  }
  return std::nullopt;
}
}  // namespace

auto ResolveUvmPath() -> std::optional<fs::path> {
  std::error_code ec;

  std::optional<fs::path> installed;
  const fs::path exe = fs::read_symlink("/proc/self/exe", ec);
  if (!ec) {
    fs::path candidate = exe.parent_path().parent_path() / "share" /
                         "verihogg-lint" / "uvm" / "src";
    if (fs::is_directory(candidate, ec)) {
      installed = candidate;
    }
  }

  if (auto env = GetEnvVar("VERIHOGG_UVM_PATH")) {
    fs::path p{*env};
    if (fs::is_directory(p, ec)) {
      if (installed.has_value()) {
        std::cerr << "warning: VERIHOGG_UVM_PATH overrides built-in UVM (" << p
                  << ")\n";
      }
      return p;
    }
    std::cerr << "error: VERIHOGG_UVM_PATH=" << p << " is not a directory\n";
    return std::nullopt;
  }

  return installed;
}

}  // namespace uvm
