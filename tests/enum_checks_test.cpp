#include <Surelog/API/Surelog.h>
#include <Surelog/CommandLine/CommandLineParser.h>
#include <Surelog/Design/Design.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/ErrorReporting/ErrorDefinition.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <gtest/gtest.h>

#include <array>
#include <exception>
#include <filesystem>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

#include "main/lint_rules.h"
#include "rules/duplicate_enum_literal.h"
#include "utils/init.h"

namespace SL = SURELOG;
namespace fs = std::filesystem;

namespace {

auto BasePath() -> fs::path {
  return fs::current_path() / ".." / ".." / "tests" / "EnumChecks";
}

auto getDesignFromPath(const fs::path& path, SL::ErrorContainer* errors,
                       SL::SymbolTable* symbols) -> SL::Design* {
  const auto clp = std::make_unique<SURELOG::CommandLineParser>(errors, symbols,
                                                                false, false);
  InitCommandLineParser(clp.get());

  const std::string path_str = path.string();
  std::array<const char*, 2> argv = {"", path_str.c_str()};
  if (!clp->parseCommandLine(2, argv.data())) {
    std::cerr << "Can't parse command line" << "\n";
  }

  try {
    const auto compiler = start_compiler(clp.get());
    return get_design(compiler);
  } catch (const std::exception& e) {
    std::cerr << "Compiler error: " << e.what() << '\n';
    return nullptr;
  }
}

void testCheckWithNoErrorsExpected(
    const fs::path& tests_path,
    const std::function<void(SL::Design*, SL::ErrorContainer*,
                             SL::SymbolTable*)>& check_func) {
  for (auto& file_path : fs::directory_iterator{tests_path}) {
    std::cout << "TESTING FILE:" << file_path << "\n";
    auto symbols = std::make_unique<SURELOG::SymbolTable>();
    auto errors = std::make_unique<SURELOG::ErrorContainer>(symbols.get());

    SL::Design* design =
        getDesignFromPath(file_path, errors.get(), symbols.get());
    check_func(design, errors.get(), symbols.get());
    errors->printMessages();
    ASSERT_EQ(errors.get()->getErrors().size(), 0);
  }
}

void testCheckWithErrorsExpected(
    const fs::path& tests_path, verihogg_lint::LintId errorIdExpected,
    const std::unordered_set<SURELOG::ErrorDefinition::ErrorType>& ignoreList,
    const std::function<void(SL::Design*, SL::ErrorContainer*,
                             SL::SymbolTable*)>& check_func) {
  for (auto& file_path : fs::directory_iterator{tests_path}) {
    std::cout << "TESTING FILE:" << file_path << "\n";
    auto symbols = std::make_unique<SURELOG::SymbolTable>();
    auto errors = std::make_unique<SURELOG::ErrorContainer>(symbols.get());

    SL::Design* design =
        getDesignFromPath(file_path, errors.get(), symbols.get());
    check_func(design, errors.get(), symbols.get());
    errors->printMessages();

    auto errorVector = errors.get()->getErrors();
    ASSERT_NE(errorVector.size(), 0);
    bool hasError = false;
    for (auto& error : errorVector) {
      const SURELOG::ErrorDefinition::ErrorType type = error.getType();
      if (ignoreList.count(type) > 0) {
        continue;
      }
      hasError = true;
      ASSERT_EQ(type, errorIdExpected);
    }
    ASSERT_EQ(hasError, true);
  }
}

TEST(DuplicateEnumLiteralTest, NoError) {
  const fs::path tests_path{BasePath() / "DuplicateEnumLiteral" / "NoError"};
  testCheckWithNoErrorsExpected(tests_path, CheckDuplicateEnumLiteral);
}

TEST(DuplicateEnumLiteralTest, RaiseError) {
  const fs::path tests_path{BasePath() / "DuplicateEnumLiteral" / "RaiseError"};
  const std::unordered_set<SURELOG::ErrorDefinition::ErrorType> ignoreList{};
  testCheckWithErrorsExpected(tests_path,
                              verihogg_lint::LINT_DUPLICATE_ENUM_LITERAL,
                              ignoreList, CheckDuplicateEnumLiteral);
}

}  // namespace

auto main(int argc, char** argv) -> int {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
