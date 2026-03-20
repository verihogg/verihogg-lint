#pragma once

#include <vector>

namespace cli {

inline constexpr const char* kVersion = "0.1.0";

struct Options {
  bool show_help = false;
  bool show_version = false;
  bool show_rules = false;
  bool show_surelog_help = false;

  std::vector<const char*> surelog_args;
};

auto parse_args(int argc, const char** argv) -> Options;

void print_help(const char* program_name);
void print_version();
void print_rules();

}  // namespace cli