#include "utils/uvm_path.h"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string>

namespace uvm {

namespace fs = std::filesystem;

namespace {
auto GetEnvVar(const char* name) -> std::optional<std::string> {
  const char* val = std::getenv(name);  // NOLINT(concurrency-mt-unsafe)
  if (val == nullptr) {
    return std::nullopt;
  }
  return std::string{val};
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

  if (auto env = GetEnvVar("VERIHOGG_UVM_PATH"); env.has_value()) {
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

auto IsUvmFile(std::string_view pathStr, const fs::path& uvmDir) -> bool {
  if (pathStr.empty()) {
    return false;
  }
  std::error_code ec;
  const auto rel = fs::relative(fs::path{pathStr}, uvmDir, ec);
  return !ec && !rel.empty() && rel.native().front() != '.';
}

}  // namespace uvm
