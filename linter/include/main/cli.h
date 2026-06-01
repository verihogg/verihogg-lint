#pragma once

#include <filesystem>
#include <gsl/span>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

namespace cli {

inline constexpr const char* kVersion = "0.2.0";

enum class UvmMode : std::uint8_t {
  None,
  Builtin,
  Custom,
};

struct Options {
  bool dump_config = false;
  bool show_help = false;
  bool show_version = false;
  bool show_rules = false;
  bool show_surelog_help = false;
  bool show_uvm_version = false;

  UvmMode uvm_mode = UvmMode::None;
  std::optional<std::filesystem::path> uvm_path;

  bool apply_fixes = false;
  bool fix_dry_run = false;
  std::string dry_run_file;
  bool show_suggestions = false;
  std::string backup_suffix;

  std::filesystem::path config_file;
  std::vector<const char*> surelog_args;

  std::string error_message;
};

auto ParseArgs(gsl::span<const char*> args) -> Options;
void DumpConfig();

void PrintHelp(const char* programName);
void PrintVersion();
void PrintUvmVersion();
void PrintRules();

}  // namespace cli