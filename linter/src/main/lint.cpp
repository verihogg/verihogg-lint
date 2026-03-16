#include <iostream>
#include <memory>

#include "Surelog/API/Surelog.h"
#include "Surelog/CommandLine/CommandLineParser.h"
#include "main/rule_dispatcher.h"

namespace SL = SURELOG;

auto main(int argc, const char** argv) -> int {
  auto symbolTable = std::make_unique<SL::SymbolTable>();
  auto errors = std::make_unique<SL::ErrorContainer>(symbolTable.get());
  auto clp = std::make_unique<SL::CommandLineParser>(
      errors.get(), symbolTable.get(), false, false);

  clp->noPython();
  clp->setParse(true);
  clp->setCompile(true);
  clp->setElaborate(true);
  clp->setwritePpOutput(true);
  clp->setCacheAllowed(false);
  clp->setFilterInfo();
  clp->setFilterNote();
  clp->setFilterWarning();

  bool success = clp->parseCommandLine(argc, argv);
  SL::Design* theDesign = nullptr;
  SL::scompiler* compiler = nullptr;
  vpiHandle uhdmDesign = nullptr;

  if (success && !clp->help()) {
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
    std::cerr << "No design created" << std::endl;
    return 1;
  }

  RunAllRulesOnDesign(theDesign, uhdmDesign, errors.get(), symbolTable.get());

  errors->printMessages(clp->muteStdout());

  uint32_t errorCount = errors->getErrors().size();

  if (errorCount == 0) {
    std::cout << "Lint completed successfully. No issues found." << std::endl;
  } else {
    std::cout << "Lint finished with " << errorCount << " error(s)."
              << std::endl;
  }

  if (success && !clp->help()) {
    shutdown_compiler(compiler);
  }

  return 0;
}
