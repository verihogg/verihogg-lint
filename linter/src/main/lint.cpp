#include <Surelog/API/Surelog.h>
#include <Surelog/CommandLine/CommandLineParser.h>
#include <Surelog/ErrorReporting/ErrorDefinition.h>

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <iostream>
#include <memory>
#include <span>
#include <vector>

#include "main/cli.h"
#include "main/lint_rules.h"
#include "main/rule_dispatcher.h"
#include "uhdm/vpi_user.h"

namespace SL = SURELOG;

auto main(int argc, const char** argv) -> int {
  assert(argc >= 0);
  const auto args = std::span{argv, static_cast<size_t>(argc)};
  const cli::Options kOpts = cli::ParseArgs(args);

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

  clp->noPython();
  clp->setParse(true);
  clp->setCompile(true);
  clp->setElaborate(true);
  clp->setwritePpOutput(true);
  clp->setCacheAllowed(false);
  clp->setFilterInfo();
  clp->setFilterNote();
  clp->setFilterWarning();

  if (kOpts.show_surelog_help) {
    std::array<const char*, 2> helpArgv = {args[0], "--help"};
    clp->parseCommandLine(static_cast<int>(helpArgv.size()), helpArgv.data());
    return 0;
  }

  std::vector<const char*> slArgv = kOpts.surelog_args;
  if (kOpts.show_surelog_help) {
    slArgv.push_back("--help");
  }

  const int kSlArgc = static_cast<int>(slArgv.size());
  const bool kSuccess = clp->parseCommandLine(kSlArgc, slArgv.data());

  if (clp->help()) {
    return 0;
  }

  if (!kSuccess) {
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

  RunAllRulesOnDesign(theDesign, uhdmDesign, errors.get(), symbolTable.get());

  errors->printMessages(clp->muteStdout());

  const uint32_t kErrorCount = errors->getErrors().size();

  if (kErrorCount == 0) {
    std::cout << "Lint completed successfully. No issues found." << '\n';
  } else {
    std::cout << "Lint finished with " << kErrorCount << " error(s)." << '\n';
  }

  if (compiler != nullptr) {
    shutdown_compiler(compiler);
  }

  return 0;
}
