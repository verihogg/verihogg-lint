#include <Surelog/API/Surelog.h>
#include <Surelog/CommandLine/CommandLineParser.h>
#include <Surelog/ErrorReporting/ErrorDefinition.h>
#include <uhdm/vpi_user.h>

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <exception>
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

  std::vector<const char*> slArgv = kOpts.surelog_args;

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

  if (kSuccess) {
    try {
      compiler = start_compiler(clp.get());
      theDesign = get_design(compiler);
      uhdmDesign = get_uhdm_design(compiler);
    } catch (const std::exception& e) {
      std::cerr << "Compiler error: " << e.what() << '\n';
      return 1;
    }
  }

  if (theDesign == nullptr && uhdmDesign == nullptr) {
    std::cerr << "No design created" << '\n';
    return 1;
  }

  // Lazily create autofix components based on which flags are active.
  const bool kNeedCollector = kOpts.show_suggestions || kOpts.apply_fixes ||
                              !kOpts.export_fixes.empty();
  const bool kNeedReplacements =
      kOpts.apply_fixes || !kOpts.export_fixes.empty();
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
                      kOpts.config_file, autofix_ptr);

  errors->printMessages(clp->muteStdout());

  const uint32_t kErrorCount = errors->getErrors().size();

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
    file_repls->printDiff();
  }

  if (!kOpts.export_fixes.empty() && file_repls && !file_repls->empty()) {
    if (file_repls->exportToYaml(kOpts.export_fixes)) {
      std::cout << "Fixes exported to: " << kOpts.export_fixes << "\n";
    } else {
      std::cerr << "autofix: failed to export fixes to: " << kOpts.export_fixes
                << "\n";
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
    if (!failed_files.empty()) {
      std::cerr << "autofix: warning: " << failed_files.size()
                << " file(s) failed — design may be in inconsistent state\n";
    }
    if (!fixed_files.empty()) {
      std::cout << "Applied " << fixed_files.size() << " fix(es).\n";
    }
  }

  if (compiler != nullptr) {
    shutdown_compiler(compiler);
  }

  return (kErrorCount == 0) ? 0 : 1;
}
