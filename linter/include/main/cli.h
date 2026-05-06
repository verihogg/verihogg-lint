#pragma once

#include <filesystem>
#include <gsl/span>
#include <string>
#include <string_view>
#include <unordered_set>
#include <string>
#include <vector>

namespace cli {

inline constexpr const char* kVersion = "0.2.0";

struct Options {
  bool dump_config = false;
  bool show_help = false;
  bool show_version = false;
  bool show_rules = false;
  bool show_surelog_help = false;

  bool apply_fixes = false;
  bool fix_dry_run = false;
  bool show_suggestions = false;
  std::string export_fixes;
  std::string import_fixes;
  std::string backup_suffix;

  std::filesystem::path config_file;
  std::vector<const char*> surelog_args;

  std::string error_message;
};

auto ParseArgs(gsl::span<const char*> args) -> Options;
void DumpConfig();

void PrintHelp(const char* programName);
void PrintVersion();
void PrintRules();

}  // namespace cli