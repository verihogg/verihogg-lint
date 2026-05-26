#include <Surelog/API/Surelog.h>
#include <Surelog/CommandLine/CommandLineParser.h>
#include <Surelog/Design/Design.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/ErrorReporting/ErrorDefinition.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <memory>
#include <string>

#include "main/lint_rules.h"
#include "rules/duplicate_checker.h"
#include "rules/illegal_checker_instance.h"
#include "rules/undeclared_checker.h"
#include "utils/init.h"

namespace fs = std::filesystem;
namespace SL = SURELOG;

auto basePath() -> fs::path {
  return fs::current_path() / ".." / ".." / "tests" / "CheckerChecks";
}

auto compileFile(const fs::path& path, SL::SymbolTable* symbols,
                 SL::ErrorContainer* errors)
    -> std::pair<SL::Design*, SL::FileContent*> {
  auto clp =
      std::make_unique<SL::CommandLineParser>(errors, symbols, false, false);
  InitCommandLineParser(clp.get());
  std::string path_str = path.string();
  std::array<const char*, 2> argv = {"", path_str.c_str()};
  if (!clp->parseCommandLine(2, argv.data())) {
    return {nullptr, nullptr};
  }
  auto compiler = start_compiler(clp.get());
  auto design = get_design(compiler);
  if (!design) {
    return {nullptr, nullptr};
  }
  auto fcs = design->getAllFileContents();
  SL::FileContent* fc = fcs.empty() ? nullptr : fcs.begin()->second;
  return {design, fc};
}

void expectNoErrors(
    const fs::path& dir,
    const std::function<void(SL::Design*, SL::FileContent*, SL::ErrorContainer*,
                             SL::SymbolTable*)>& check) {
  for (auto& entry : fs::directory_iterator(dir)) {
    if (!entry.is_regular_file()) {
      continue;
    }
    auto symbols = std::make_unique<SL::SymbolTable>();
    auto errors = std::make_unique<SL::ErrorContainer>(symbols.get());
    auto [design, fc] = compileFile(entry.path(), symbols.get(), errors.get());
    ASSERT_NE(design, nullptr);
    check(design, fc, errors.get(), symbols.get());
    errors->printMessages();
    EXPECT_EQ(errors->getErrors().size(), 0);
  }
}

void expectError(
    const fs::path& dir, verihogg_lint::LintIdEnum expected,
    const std::function<void(SL::Design*, SL::FileContent*, SL::ErrorContainer*,
                             SL::SymbolTable*)>& check) {
  for (auto& entry : fs::directory_iterator(dir)) {
    if (!entry.is_regular_file()) {
      continue;
    }
    auto symbols = std::make_unique<SL::SymbolTable>();
    auto errors = std::make_unique<SL::ErrorContainer>(symbols.get());
    auto [design, fc] = compileFile(entry.path(), symbols.get(), errors.get());
    ASSERT_NE(design, nullptr);
    check(design, fc, errors.get(), symbols.get());
    errors->printMessages();
    bool found = false;
    for (auto& err : errors->getErrors()) {
      if (static_cast<int>(err.getType()) == static_cast<int>(expected)) {
        found = true;
        break;
      }
    }
    EXPECT_TRUE(found);
  }
}

void checkDuplicate(SL::Design* /*unused*/, SL::FileContent* fc,
                    SL::ErrorContainer* err, SL::SymbolTable* sym) {
  CheckDuplicateChecker(fc, err, sym);
}

void checkIllegal(SL::Design* /*unused*/, SL::FileContent* fc,
                  SL::ErrorContainer* err, SL::SymbolTable* sym) {
  CheckIllegalCheckerInstance(fc, err, sym);
}

void checkUndeclared(SL::Design* design, SL::FileContent* /*unused*/,
                     SL::ErrorContainer* err, SL::SymbolTable* sym) {
  CheckUndeclaredChecker(design, err, sym);
}

TEST(DuplicateChecker, NoError) {
  expectNoErrors(basePath() / "DuplicateChecker" / "NoError", checkDuplicate);
}

TEST(DuplicateChecker, RaiseError) {
  expectError(basePath() / "DuplicateChecker" / "RaiseError",
              verihogg_lint::LINT_DUPLICATE_CHECKER, checkDuplicate);
}

TEST(IllegalCheckerInstance, NoError) {
  expectNoErrors(basePath() / "IllegalCheckerInstance" / "NoError",
                 checkIllegal);
}

TEST(IllegalCheckerInstance, RaiseError) {
  expectError(basePath() / "IllegalCheckerInstance" / "RaiseError",
              verihogg_lint::LINT_ILLEGAL_CHECKER_INSTANCE, checkIllegal);
}

TEST(UndeclaredChecker, NoError) {
  expectNoErrors(basePath() / "UndeclaredChecker" / "NoError", checkUndeclared);
}

TEST(UndeclaredChecker, RaiseError) {
  expectError(basePath() / "UndeclaredChecker" / "RaiseError",
              verihogg_lint::LINT_UNDECLARED_CHECKER, checkUndeclared);
}

auto main(int argc, char** argv) -> int {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
