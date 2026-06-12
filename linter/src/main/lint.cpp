#include <Surelog/API/Surelog.h>
#include <Surelog/CommandLine/CommandLineParser.h>
#include <Surelog/Common/FileSystem.h>
#include <Surelog/ErrorReporting/Error.h>
#include <Surelog/ErrorReporting/ErrorDefinition.h>
#include <uhdm/vpi_user.h>

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <fstream>
#include <gsl/span>
#include <iostream>
#include <memory>
#include <vector>

#include "fix/replacement.h"
#include "fix/source_manager.h"
#include "fix/suggestion_printer.h"
#include "main/cli.h"
#include "main/lint_diagnostics.h"
#include "main/lint_rules.h"
#include "main/rule_dispatcher.h"
#include "utils/init.h"
#include "utils/uvm_path.h"

namespace SL = SURELOG;

auto main(int argc, const char** argv) -> int {
  assert(argc >= 0);
  const auto args = gsl::span{argv, static_cast<size_t>(argc)};
  const cli::Options kOpts = cli::ParseArgs(args);

  if (!kOpts.error_message.empty()) {
    std::cerr << kOpts.error_message << '\n'
              << "Try '" << args[0] << " --help' for usage.\n";
    return 1;
  }

  if (kOpts.dump_config) {
    cli::DumpConfig();
    return 0;
  }

  if (kOpts.show_version) {
    cli::PrintVersion();
    return 0;
  }
  if (kOpts.show_help) {
    cli::PrintHelp(args[0]);
    return 0;
  }
  if (kOpts.show_rules) {
    cli::PrintRules();
    return 0;
  }

  if (kOpts.show_uvm_version) {
    cli::PrintUvmVersion();
    return 0;
  }

  auto symbolTable = std::make_unique<SL::SymbolTable>();
  auto errors = std::make_unique<SL::ErrorContainer>(symbolTable.get());
  auto clp = std::make_unique<SL::CommandLineParser>(
      errors.get(), symbolTable.get(), false, false);

  SL::ErrorDefinition::init();
  verihogg_lint::RegisterLintRules();

  InitCommandLineParser(clp.get());

  if (kOpts.show_surelog_help) {
    std::array<const char*, 2> helpArgv = {args[0], "--help"};
    clp->parseCommandLine(static_cast<int>(helpArgv.size()), helpArgv.data());
    return 0;
  }

  struct UvmSetup {
    std::filesystem::path dir;
    std::string incdir_arg;
    std::string pkg_arg;
  };

  auto ResolveUvm = [](const cli::Options& opts) -> std::optional<UvmSetup> {
    if (opts.uvm_mode == cli::UvmMode::Custom) {
      if (!opts.uvm_path.has_value()) {
        return std::nullopt;
      }
      std::error_code ec;
      if (!std::filesystem::is_directory(*opts.uvm_path, ec)) {
        std::cerr << "error: UVM path '" << *opts.uvm_path
                  << "' is not a directory\n";
        return std::nullopt;
      }
      const std::filesystem::path& dir = *opts.uvm_path;
      return UvmSetup{.dir = dir,
                      .incdir_arg = "+incdir+" + dir.string(),
                      .pkg_arg = (dir / "uvm_pkg.sv").string()};
    }
#ifndef UVM_BUILTIN_AVAILABLE
    std::cerr << "error: --uvm requires built-in UVM support.\n"
              << "  This binary was built without UVM.\n"
              << "  Use --uvm=<path> to specify a UVM library path.\n";
    return std::nullopt;
#else
    auto maybe = uvm::ResolveUvmPath();
    if (!maybe.has_value()) {
      std::cerr << "error: built-in UVM not found.\n"
                << "  Set VERIHOGG_UVM_PATH or use --uvm=<path>.\n";
      return std::nullopt;
    }
    const std::filesystem::path dir = std::move(*maybe);
    return UvmSetup{.dir = dir,
                    .incdir_arg = "+incdir+" + dir.string(),
                    .pkg_arg = (dir / "uvm_pkg.sv").string()};
#endif
  };

  std::vector<const char*> slArgv = kOpts.surelog_args;
  std::optional<UvmSetup> uvm_setup;
  std::optional<std::filesystem::path> uvm_dir;

  if (kOpts.uvm_mode != cli::UvmMode::None) {
    uvm_setup = ResolveUvm(kOpts);
    if (!uvm_setup.has_value()) {
      return 1;
    }
    uvm_dir = uvm_setup->dir;
    slArgv.insert(slArgv.begin() + 1, uvm_setup->incdir_arg.c_str());
    slArgv.insert(slArgv.begin() + 2, uvm_setup->pkg_arg.c_str());
  }

  const int kSlArgc = static_cast<int>(slArgv.size());
  const bool kSuccess = clp->parseCommandLine(kSlArgc, slArgv.data());

  if (clp->help()) {
    return 0;
  }

  if (!kSuccess) {
    errors->printMessages(clp->muteStdout());
    std::cerr << "Try '" << args[0] << " --help' for usage.\n";
    return 1;
  }

  if (clp->getSourceFiles().empty()) {
    std::cerr << args[0] << ": no input files\n"
              << "Try '" << args[0] << " --help' for usage.\n";
    return 1;
  }

  SL::Design* theDesign = nullptr;
  SL::scompiler* compiler = nullptr;
  vpiHandle uhdmDesign = nullptr;

  try {
    compiler = start_compiler(clp.get());
    theDesign = get_design(compiler);
    uhdmDesign = get_uhdm_design(compiler);
  } catch (const std::exception& e) {
    std::cerr << "Compiler error: " << e.what() << '\n';
    return 1;
  }

  if (theDesign == nullptr && uhdmDesign == nullptr) {
    std::cerr << "No design created" << '\n';
    return 1;
  }

  const bool kNeedCollector =
      kOpts.show_suggestions || kOpts.apply_fixes || kOpts.fix_dry_run;
  const bool kNeedReplacements = kOpts.apply_fixes || kOpts.fix_dry_run;
  const bool kNeedSourceMgr = kNeedReplacements || kOpts.show_suggestions;

  std::unique_ptr<LintDiagnosticCollector> collector;
  std::unique_ptr<FileReplacements> file_repls;
  std::unique_ptr<FixSourceManager> source_mgr;

  if (kNeedCollector) {
    collector = std::make_unique<LintDiagnosticCollector>();
  }
  if (kNeedReplacements) {
    file_repls = std::make_unique<FileReplacements>();
  }
  if (kNeedSourceMgr) {
    source_mgr = std::make_unique<FixSourceManager>();
  }

  AutofixContext autofix_ctx;
  autofix_ctx.collector = collector.get();
  autofix_ctx.replacements = file_repls.get();
  autofix_ctx.source_mgr = source_mgr.get();

  AutofixContext* autofix_ptr = kNeedCollector ? &autofix_ctx : nullptr;

  RunAllRulesOnDesign(theDesign, uhdmDesign, errors.get(), symbolTable.get(),
                      kOpts.config_file, autofix_ptr, uvm_dir);

  auto printErrors = std::make_unique<SL::ErrorContainer>(symbolTable.get());
  printErrors->registerCmdLine(clp.get());
  for (auto err : errors->getErrors()) {
    bool fromUvm = false;
    if (uvm_dir.has_value() && !err.getLocations().empty()) {
      const std::string_view path = SL::FileSystem::getInstance()->toPath(
          err.getLocations().front().m_fileId);
      fromUvm = uvm::IsUvmFile(path, *uvm_dir);
    }
    if (!fromUvm) {
      printErrors->addError(err, false);
    }
  }

  printErrors->printMessages(clp->muteStdout());

  const uint32_t kErrorCount = printErrors->getErrors().size();

  if (kErrorCount == 0) {
    std::cout << "Lint completed successfully. No issues found.\n";
  } else {
    std::cout << "Lint finished with " << kErrorCount << " error(s).\n";
  }

  if (kOpts.show_suggestions && collector && collector->hasFixable()) {
    SuggestionPrinter::print(collector->all(), source_mgr.get(),
                             /*show_diff=*/true);
  }

  if (kOpts.fix_dry_run && file_repls && !file_repls->empty()) {
    if (kOpts.dry_run_file.empty()) {
      std::cout << "\n";
      file_repls->printDiff(std::cout);
    } else {
      std::ofstream patch_out(kOpts.dry_run_file,
                              std::ios::binary | std::ios::trunc);
      if (!patch_out.is_open()) {
        std::cerr << "autofix: cannot open patch output file: "
                  << kOpts.dry_run_file << "\n";
        return 1;
      }
      file_repls->printDiff(patch_out);
      if (!patch_out.good()) {
        std::cerr << "autofix: write error on patch file: "
                  << kOpts.dry_run_file << "\n";
        return 1;
      }
      std::cout << "Patch written to: " << kOpts.dry_run_file << "\n";
    }
  }

  if (kOpts.apply_fixes && file_repls && !file_repls->empty()) {
    std::vector<std::string> fixed_files;
    std::vector<std::string> failed_files;

    file_repls->applyAll(&fixed_files, &failed_files, kOpts.backup_suffix);

    for (const auto& f : fixed_files) {
      std::cout << "Fixed: " << f << "\n";
    }
    for (const auto& f : failed_files) {
      std::cerr << "autofix: failed to fix: " << f << "\n";
    }

    if (!fixed_files.empty()) {
      std::cout << "Applied " << file_repls->totalCount() << " fix(es) across "
                << fixed_files.size() << " file(s).\n";
    }

    if (!failed_files.empty()) {
      std::cerr << "WARNING: " << failed_files.size()
                << " file(s) could not be fixed. "
                << "Design may be in partially-fixed state. "
                << "Check file permissions and re-run.\n";
    }
  }

  if (compiler != nullptr) {
    shutdown_compiler(compiler);
  }

  return (kErrorCount == 0) ? 0 : 1;
}
