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
#include "rules/always_event_control.h"
#include "rules/disable_fork.h"
#include "rules/fork_disable.h"
#include "rules/fork_variable_access.h"
#include "rules/named_fork.h"
#include "utils/init.h"

namespace SL = SURELOG;
namespace fs = std::filesystem;

namespace {

auto BasePath() -> fs::path {
  return fs::current_path() / ".." / ".." / "tests" / "ProcessesChecks";
}

auto getFileContentFromPath(const fs::path& path, SL::ErrorContainer* errors,
                            SL::SymbolTable* symbols) -> SL::FileContent* {
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
    const auto vec = get_design(compiler)->getAllFileContents();
    return vec.at(0).second;
  } catch (const std::exception& e) {
    std::cerr << "Compiler error: " << e.what() << '\n';
    return nullptr;
  }
}

template <typename T>
auto ToErrorType(T id) -> SURELOG::ErrorDefinition::ErrorType {
  return static_cast<SURELOG::ErrorDefinition::ErrorType>(id);
}

void testCheckWithNoErrorsExpected(
    const fs::path& tests_path,
    const std::function<void(const SL::FileContent*, SL::ErrorContainer*,
                             SL::SymbolTable*)>& check_func) {
  for (auto& file_path : fs::directory_iterator{tests_path}) {
    std::cout << "TESTING FILE:" << file_path << "\n";
    auto symbols = std::make_unique<SURELOG::SymbolTable>();
    auto errors = std::make_unique<SURELOG::ErrorContainer>(symbols.get());

    const SL::FileContent* fC =
        getFileContentFromPath(file_path, errors.get(), symbols.get());
    check_func(fC, errors.get(), symbols.get());
    errors->printMessages();
    ASSERT_EQ(errors.get()->getErrors().size(), 0);
  }
}

void testCheckWithErrorsExpected(
    const fs::path& tests_path,
    SURELOG::ErrorDefinition::ErrorType errorIdExpected,
    const std::unordered_set<SURELOG::ErrorDefinition::ErrorType>& ignoreList,
    const std::function<void(const SL::FileContent*, SL::ErrorContainer*,
                             SL::SymbolTable*)>& check_func) {
  for (auto& file_path : fs::directory_iterator{tests_path}) {
    std::cout << "TESTING FILE:" << file_path << "\n";
    auto symbols = std::make_unique<SURELOG::SymbolTable>();
    auto errors = std::make_unique<SURELOG::ErrorContainer>(symbols.get());

    const SL::FileContent* fC =
        getFileContentFromPath(file_path, errors.get(), symbols.get());
    check_func(fC, errors.get(), symbols.get());
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

TEST(AlwaysEventControlTest, NoError) {
  const fs::path tests_path{BasePath() / "AlwaysEventControl" / "NoError"};
  testCheckWithNoErrorsExpected(tests_path, CheckAlwaysEventControl);
}

TEST(AlwaysEventControlTest, RaiseError) {
  const fs::path tests_path{BasePath() / "AlwaysEventControl" / "RaiseError"};
  const std::unordered_set<SURELOG::ErrorDefinition::ErrorType> ignoreList{};
  testCheckWithErrorsExpected(
      tests_path, ToErrorType(verihogg_lint::LINT_ALWAYS_EVENT_CONTROL),
      ignoreList, CheckAlwaysEventControl);
}

TEST(NamedForkTest, NoError) {
  const fs::path tests_path{BasePath() / "NamedFork" / "NoError"};
  testCheckWithNoErrorsExpected(tests_path, CheckNamedFork);
}

TEST(NamedForkTest, RaiseError) {
  const fs::path tests_path{BasePath() / "NamedFork" / "RaiseError"};
  const std::unordered_set<SURELOG::ErrorDefinition::ErrorType> ignoreList{};
  testCheckWithErrorsExpected(tests_path,
                              ToErrorType(verihogg_lint::LINT_NAMED_FORK),
                              ignoreList, CheckNamedFork);
}

TEST(ForkVariableAccessTest, NoError) {
  const fs::path tests_path{BasePath() / "ForkVariableAccess" / "NoError"};
  testCheckWithNoErrorsExpected(tests_path, CheckForkVariableAccess);
}

TEST(ForkVariableAccessTest, RaiseError) {
  const fs::path tests_path{BasePath() / "ForkVariableAccess" / "RaiseError"};
  const std::unordered_set<SURELOG::ErrorDefinition::ErrorType> ignoreList{};
  testCheckWithErrorsExpected(
      tests_path, ToErrorType(verihogg_lint::LINT_FORK_VARIABLE_ACCESS),
      ignoreList, CheckForkVariableAccess);
}

TEST(ForkDisableTest, NoError) {
  const fs::path tests_path{BasePath() / "ForkDisable" / "NoError"};
  testCheckWithNoErrorsExpected(tests_path, CheckForkDisable);
}

TEST(ForkDisableTest, RaiseError) {
  const fs::path tests_path{BasePath() / "ForkDisable" / "RaiseError"};
  const std::unordered_set<SURELOG::ErrorDefinition::ErrorType> ignoreList{};
  testCheckWithErrorsExpected(tests_path,
                              ToErrorType(verihogg_lint::LINT_FORK_DISABLE),
                              ignoreList, CheckForkDisable);
}

TEST(DisableForkTest, NoError) {
  const fs::path tests_path{BasePath() / "DisableFork" / "NoError"};
  testCheckWithNoErrorsExpected(tests_path, CheckDisableFork);
}

TEST(DisableForkTest, RaiseError) {
  const fs::path tests_path{BasePath() / "DisableFork" / "RaiseError"};
  const std::unordered_set<SURELOG::ErrorDefinition::ErrorType> ignoreList{};
  testCheckWithErrorsExpected(tests_path,
                              ToErrorType(verihogg_lint::LINT_DISABLE_FORK),
                              ignoreList, CheckDisableFork);
}

}  // namespace

auto main(int argc, char** argv) -> int {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
