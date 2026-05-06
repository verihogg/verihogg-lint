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

constexpr size_t CONFIG_FLAG_LEN = 13;
inline constexpr std::string_view kConfigFilePrefix = "--config-file=";
inline constexpr std::string_view kExportFixesPrefix = "--export-fixes=";
inline constexpr std::string_view kImportFixesPrefix = "--apply-fixes-from=";
inline constexpr std::string_view kBackupSuffixPrefix = "--backup-suffix=";

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
    } else if (std::strncmp(arg, "--config-file=", CONFIG_FLAG_LEN + 1) == 0) {
      const std::string config_file{
          std::string_view{arg}.substr(CONFIG_FLAG_LEN + 1)};
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

    } else if (std::strcmp(arg, "--show-suggestions") == 0) {
      opts.show_suggestions = true;

    } else if (std::strncmp(
                   arg, "--export-fixes=", kExportFixesPrefix.size()) == 0) {
      std::string_view sv(arg);
      opts.export_fixes = std::string(sv.substr(kExportFixesPrefix.size()));

    } else if (std::strncmp(arg, "--apply-fixes-from=",
                            kImportFixesPrefix.size()) == 0) {
      std::string_view sv(arg);
      opts.import_fixes = std::string(sv.substr(kImportFixesPrefix.size()));

    } else if (std::strncmp(
                   arg, "--backup-suffix=", kBackupSuffixPrefix.size()) == 0) {
      std::string_view sv(arg);
      opts.backup_suffix = std::string(sv.substr(kBackupSuffixPrefix.size()));

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
    << "  --fix                         Apply all fixable suggestions automatically\n"
    << "  --fix-dry-run                 Show diff in stdout without modifying files\n"
    << "  --show-suggestions            Print fix hints with before/after snippets\n"
    << "  --export-fixes=<file>         Export fixes to YAML without applying\n"
    << "  --apply-fixes-from=<file>     Apply fixes from a previously exported YAML\n"
    << "  --backup-suffix=<suffix>      Save originals before --fix (e.g. .bak)\n"
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

  for (const auto& rule : RuleInfo::globalRules) {
    std::cout << "\n  " << rule.idName << "\n"
              << "      " << rule.description << "\n";
  }
}
}  // namespace cli