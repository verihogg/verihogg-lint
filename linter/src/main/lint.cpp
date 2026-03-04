#include <iostream>
#include <memory>

#include "Surelog/API/Surelog.h"
#include "Surelog/CommandLine/CommandLineParser.h"
#include "main/rule_dispatcher.h"

using namespace SURELOG;

int main(int argc, const char** argv) {
  auto symbolTable = std::make_unique<SymbolTable>();
  auto errors = std::make_unique<ErrorContainer>(symbolTable.get());
  auto clp = std::make_unique<CommandLineParser>(errors.get(), symbolTable.get(), false, false);

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
  Design* the_design = nullptr;
  scompiler* compiler = nullptr;
  vpiHandle UHDMdesign = nullptr;

  if (success && !clp->help()) {
    try {
    compiler = start_compiler(clp.get());
    the_design = get_design(compiler);
    UHDMdesign = get_uhdm_design(compiler);
    } catch (const std::exception& e) {
      std::cerr << "Compiler error: " << e.what() << '\n';
      return 1;
    }
  }

  if (!the_design && !UHDMdesign) {
    std::cerr << "No design created" << std::endl;
    return 1;
  }

  runAllRulesOnDesign(the_design, UHDMdesign, errors.get(), symbolTable.get());

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
