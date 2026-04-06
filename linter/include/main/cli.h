#pragma once

#include <filesystem>
#include <gsl/span>
#include <vector>

namespace cli {

inline constexpr const char* kVersion = "0.1.0";

struct Options {
  bool dump_config = false;
  bool show_help = false;
  bool show_version = false;
  bool show_rules = false;
  bool show_surelog_help = false;

  std::filesystem::path config_file;
  std::vector<const char*> surelog_args;
};

auto ParseArgs(gsl::span<const char*> args) -> Options;

void PrintHelp(const char* programName);
void PrintVersion();
void PrintRules();

}  // namespace cli
