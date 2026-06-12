#include "main/cli.h"

#include <cstring>
#include <filesystem>
#include <gsl/span>
#include <iostream>
#include <string>
#include <string_view>

#include "main/rule_dispatcher.h"
#include "main/surelog_flags.h"

namespace cli {

inline constexpr std::string_view kConfigFilePrefix = "--config-file=";
inline constexpr std::string_view kBackupSuffixPrefix = "--backup-suffix=";
inline constexpr std::string_view kDryRunPrefix = "--fix-dry-run=";
constexpr std::string_view kUvmPrefix = "--uvm=";

auto ParseArgs(const gsl::span<const char*> args) -> Options {
  Options opts;

  opts.surelog_args.push_back(args[0]);
  opts.surelog_args.push_back("-nobuiltin");

  const std::filesystem::path configFileName = DefaultConfigFileName;
  opts.config_file = std::filesystem::current_path() / configFileName;

  for (const auto arg : args.subspan(1)) {
    if (std::strcmp(arg, "--dump-config") == 0) {
      opts.dump_config = true;
      return opts;
    } else if (std::strcmp(arg, "--help") == 0 || std::strcmp(arg, "-h") == 0) {
      opts.show_help = true;
    } else if (std::strcmp(arg, "--version") == 0) {
      opts.show_version = true;
    } else if (std::strcmp(arg, "--list-rules") == 0) {
      opts.show_rules = true;
    } else if (std::strcmp(arg, "--surelog-help") == 0) {
      opts.show_surelog_help = true;
    } else if (std::strcmp(arg, "--config-file") == 0) {
      opts.error_message =
          "verihogg-lint: option '--config-file' requires '=<path>'";
      return opts;
    } else if (std::string_view{arg}.starts_with(kConfigFilePrefix)) {
      const std::string config_file{
          std::string_view{arg}.substr(kConfigFilePrefix.size())};
      const std::filesystem::path config_path{config_file};
      if (!std::filesystem::exists(config_path)) {
        opts.error_message = "verihogg-lint: cannot open config file '" +
                             config_file + "': No such file or directory";
        return opts;
      }
      opts.config_file = config_path;
    } else if (std::strcmp(arg, "--fix") == 0) {
      opts.apply_fixes = true;

    } else if (std::strcmp(arg, "--fix-dry-run") == 0) {
      opts.fix_dry_run = true;

    } else if (std::strncmp(arg, "--fix-dry-run=", kDryRunPrefix.size()) == 0) {
      opts.fix_dry_run = true;
      opts.dry_run_file = std::string_view(arg).substr(kDryRunPrefix.size());

    } else if (std::strcmp(arg, "--show-suggestions") == 0) {
      opts.show_suggestions = true;

    } else if (std::strncmp(
                   arg, "--backup-suffix=", kBackupSuffixPrefix.size()) == 0) {
      std::string_view sv(arg);
      opts.backup_suffix = std::string(sv.substr(kBackupSuffixPrefix.size()));

    } else if (std::strcmp(arg, "--uvm-version") == 0) {
      opts.show_uvm_version = true;
    } else if (std::strcmp(arg, "--uvm") == 0) {
      opts.uvm_mode = UvmMode::Builtin;
    } else if (std::string_view{arg}.starts_with(kUvmPrefix)) {
      opts.uvm_mode = UvmMode::Custom;
      opts.uvm_path = std::filesystem::path{
          std::string_view{arg}.substr(kUvmPrefix.size())};
    } else {
      opts.surelog_args.push_back(arg);
    }
  }

  if (opts.show_help || opts.show_version || opts.show_rules ||
      opts.show_surelog_help) {
    return opts;
  }

  for (size_t i = 1; i < opts.surelog_args.size();) {
    const std::string_view a{opts.surelog_args.at(i)};

    if (a == "-f" && i + 1 < opts.surelog_args.size()) {
      const std::string filelist{opts.surelog_args.at(i + 1)};
      if (!std::filesystem::exists(filelist)) {
        opts.error_message = "verihogg-lint: cannot open filelist '" +
                             filelist + "': No such file or directory";
        return opts;
      }
      i += 2;
      continue;
    }

    if (!a.empty() && a.front() != '-' && a.front() != '+' &&
        (a.ends_with(".sv") || a.ends_with(".svh") || a.ends_with(".v") ||
         a.ends_with(".vh"))) {
      const std::string source_file{a};
      if (!std::filesystem::exists(source_file)) {
        opts.error_message = "verihogg-lint: cannot open source file '" +
                             source_file + "': No such file or directory";
        return opts;
      }
      i += 1;
      continue;
    }

    const auto kind = ClassifySurelogFlag(a);
    if (kind == SurelogFlagKind::NoArg) {
      i += 1;
      continue;
    }
    if (kind == SurelogFlagKind::OneArg) {
      i += 2;
      continue;
    }
    if (kind == SurelogFlagKind::TwoArg) {
      i += 3;
      continue;
    }

    if (!a.empty() && a.front() == '-') {
      opts.error_message = std::string{"verihogg-lint: unrecognized option '"} +
                           std::string{a} + "'";
      return opts;
    }

    i += 1;
  }

  return opts;
}

void DumpConfig() {
  std::cout << "Checks:\n";
  for (auto& rule : RuleInfo::allRules) {
    std::cout << "  - " << rule.idName << "\n";
  }
  for (auto& rule : RuleInfo::globalRules) {
    std::cout << "  - " << rule.idName << "\n";
  }
}

void PrintVersion() { std::cout << "verihogg-lint " << kVersion << "\n"; }

void PrintUvmVersion() {
#ifdef UVM_BUILTIN_AVAILABLE
  std::cout << "built-in UVM: " << UVM_BUILTIN_VERSION << "\n";
#else
  std::cout << "built-in UVM: not available (built without UVM support)\n";
#endif
}

void PrintHelp(const char* programName) {
  // clang-format off
  std::cout
    << "Usage: " << programName << " [OPTIONS] <file.sv> [<file.sv>...]\n"
    << "       " << programName << " [OPTIONS] -f <filelist>\n"
    << "\n"
    << "A SystemVerilog linter with static analysis rules built on Surelog.\n"
    << "\n"
    << "OPTIONS:\n"
    << "  -h, --help          Show this help and exit\n"
    << "  --version           Show version and exit\n"
    << "  --list-rules        List all available lint rules\n"
    << "  --config-file=<path>\n"
    << "                      Use lint config from <path> (default: ./" << DefaultConfigFileName << ")\n"
    << "  --surelog-help      Show full Surelog parser/elaboration options\n"
    << "\n"
    << "AUTOFIX:\n"
    << "  --fix                         Apply all fixable suggestions in-place\n"
    << "  --fix-dry-run[=<file>]        Write a unified diff without modifying files.\n"
    << "                                Omit <file> to print to stdout.\n"
    << "                                Compatible with 'patch -p1' and 'git apply'.\n"
    << "  --show-suggestions            Print fix hints with before/after snippets\n"
    << "  --backup-suffix=<suffix>      Save originals before --fix (e.g. .bak)\n"
    << "UVM:\n"
    << "  --uvm               Add built-in UVM library to include path\n"
    << "  --uvm=<dir>         Add <dir> as UVM include path (custom version)\n"
    << "  --uvm-version       Show built-in UVM version and exit\n"
    << "\n"
    << "INPUT:\n"
    << "  <file>.sv           SystemVerilog source file\n"
    << "  -f <file>           Filelist (sources, includes, defines)\n"
    << "  +incdir+<dir>       Add include directory (repeatable)\n"
    << "  +define+<name>[=<value>]\n"
    << "                      Define a preprocessor macro (repeatable)\n"
    << "  -nobuiltin          Skip built-in SV classes (recommended)\n"
    << "\n"
    << "FILELIST FORMAT (.f file):\n"
    << "  Lines starting with // or # are comments.\n"
    << "  Example:\n"
    << "    +incdir+rtl/include\n"
    << "    +define+SYNTHESIS\n"
    << "    rtl/pkg/common_pkg.sv\n"
    << "    rtl/core/top.sv\n"
    << "\n"
    << "EXAMPLES:\n"
    << "  " << programName << " file.sv -nobuiltin\n"
    << "  " << programName << " -f files.f -nobuiltin\n"
    << "  " << programName << " --fix-dry-run file.sv | git apply\n"
    << "  " << programName << " --fix-dry-run=fixes.patch file.sv\n"
    << "  " << programName << " -f files.f --uvm -nobuiltin\n"
    << "  " << programName << " -f files.f --uvm=/opt/uvm/src -nobuiltin\n"
    << "  " << programName << " --list-rules\n"
    << "\n"
    << "All other flags are forwarded to Surelog (parser/elaboration).\n"
    << "Run '" << programName << " --surelog-help' for the full list.\n";
  // clang-format on
}

void PrintRules() {
  std::cout << "Available lint rules (" << RuleInfo::TotalRuleCount << "):\n";

  for (const auto& rule : RuleInfo::allRules) {
    std::cout << "\n  " << rule.idName << "\n"
              << "      " << rule.description << "\n";
  }

  for (const auto& rule : RuleInfo::fixableRules) {
    std::cout << "\n  " << rule.idName << "\n"
              << "      " << rule.description << "\n";
  }

  for (const auto& rule : RuleInfo::globalRules) {
    std::cout << "\n  " << rule.idName << "\n"
              << "      " << rule.description << "\n";
  }

  for (const auto& rule : RuleInfo::fixableGlobalRules) {
    std::cout << "\n  " << rule.idName << "\n"
              << "      " << rule.description << "\n";
  }

  for (const auto& rule : RuleInfo::uhdmRules) {
    std::cout << "\n  " << rule.idName << "\n"
              << "      " << rule.description << "\n";
  }
}
}  // namespace cli