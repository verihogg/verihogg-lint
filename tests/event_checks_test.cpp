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
#include "rules/duplicate_event.h"
#include "rules/event_singular.h"
#include "utils/init.h"

namespace SL = SURELOG;
namespace fs = std::filesystem;

namespace {

auto BasePath() -> fs::path {
  return fs::current_path() / ".." / ".." / "tests" / "EventChecks";
}

// Возвращает пару (Design*, FileContent*) для указанного файла.
// В случае ошибки возвращает {nullptr, nullptr}.
auto getDesignAndFileContent(const fs::path& path, SL::ErrorContainer* errors,
                             SL::SymbolTable* symbols)
    -> std::pair<SL::Design*, const SL::FileContent*> {
  const auto clp =
      std::make_unique<SL::CommandLineParser>(errors, symbols, false, false);
  InitCommandLineParser(clp.get());

  const std::string path_str = path.string();
  std::array<const char*, 2> argv = {"", path_str.c_str()};
  if (!clp->parseCommandLine(2, argv.data())) {
    std::cerr << "Can't parse command line" << "\n";
    return {nullptr, nullptr};
  }

  try {
    const auto compiler = start_compiler(clp.get());
    SL::Design* design = get_design(compiler);
    const auto& allFiles = design->getAllFileContents();
    if (allFiles.empty()) {
      shutdown_compiler(compiler);
      return {nullptr, nullptr};
    }
    const SL::FileContent* fC = allFiles.at(0).second;
    return {design, fC};
  } catch (const std::exception& e) {
    std::cerr << "Compiler error: " << e.what() << '\n';
    return {nullptr, nullptr};
  }
}

void testCheckWithNoErrorsExpected(
    const fs::path& tests_path,
    const std::function<void(SL::Design*, SL::ErrorContainer*,
                             SL::SymbolTable*)>& check_func) {
  for (auto& file_path : fs::directory_iterator{tests_path}) {
    std::cout << "TESTING FILE:" << file_path << "\n";
    auto symbols = std::make_unique<SL::SymbolTable>();
    auto errors = std::make_unique<SL::ErrorContainer>(symbols.get());

    auto [design, fC] =
        getDesignAndFileContent(file_path, errors.get(), symbols.get());
    if (!design || !fC) {
      FAIL() << "Failed to parse file: " << file_path;
    }

    check_func(design, errors.get(), symbols.get());
    errors->printMessages();
    ASSERT_EQ(errors->getErrors().size(), 0);
  }
}

void testCheckWithErrorsExpected(
    const fs::path& tests_path, verihogg_lint::LintId errorIdExpected,
    const std::unordered_set<SL::ErrorDefinition::ErrorType>& ignoreList,
    const std::function<void(SL::Design*, SL::ErrorContainer*,
                             SL::SymbolTable*)>& check_func) {
  for (auto& file_path : fs::directory_iterator{tests_path}) {
    std::cout << "TESTING FILE:" << file_path << "\n";
    auto symbols = std::make_unique<SL::SymbolTable>();
    auto errors = std::make_unique<SL::ErrorContainer>(symbols.get());

    auto [design, fC] =
        getDesignAndFileContent(file_path, errors.get(), symbols.get());
    if (!design || !fC) {
      FAIL() << "Failed to parse file: " << file_path;
    }

    check_func(design, errors.get(), symbols.get());
    errors->printMessages();

    auto errorVector = errors->getErrors();
    ASSERT_NE(errorVector.size(), 0);
    bool hasError = false;
    for (auto& error : errorVector) {
      const SL::ErrorDefinition::ErrorType type = error.getType();
      if (ignoreList.count(type) > 0) {
        continue;
      }
      hasError = true;
      ASSERT_EQ(type, errorIdExpected);
    }
    ASSERT_EQ(hasError, true);
  }
}

// DuplicateEvent tests
TEST(DuplicateEventTest, NoError) {
  const fs::path tests_path{BasePath() / "DuplicateEvent" / "NoError"};
  testCheckWithNoErrorsExpected(tests_path, CheckDuplicateEvents);
}

TEST(DuplicateEventTest, RaiseError) {
  const fs::path tests_path{BasePath() / "DuplicateEvent" / "RaiseError"};
  const std::unordered_set<SL::ErrorDefinition::ErrorType> ignoreList{};
  testCheckWithErrorsExpected(tests_path, verihogg_lint::LINT_DUPLICATE_EVENT,
                              ignoreList, CheckDuplicateEvents);
}

// EventSingular tests
TEST(EventSingularTest, NoError) {
  const fs::path tests_path{BasePath() / "EventSingular" / "NoError"};
  testCheckWithNoErrorsExpected(tests_path, CheckEventSingular);
}

TEST(EventSingularTest, RaiseError) {
  const fs::path tests_path{BasePath() / "EventSingular" / "RaiseError"};
  const std::unordered_set<SL::ErrorDefinition::ErrorType> ignoreList{};
  testCheckWithErrorsExpected(tests_path, verihogg_lint::LINT_EVENT_SINGULAR,
                              ignoreList, CheckEventSingular);
}

}  // namespace

auto main(int argc, char** argv) -> int {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
