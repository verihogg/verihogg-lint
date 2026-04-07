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
#include <vector>

#include "main/lint_rules.h"
#include "rules/circular_inheritance.h"
#include "rules/duplicate_class.h"
#include "rules/duplicate_constructor.h"
#include "rules/extend_class.h"
#include "rules/extend_interface_class.h"
#include "rules/extern_constraint_undeclared.h"
#include "rules/extern_function_undeclared.h"
#include "rules/extern_task_undeclared.h"
#include "rules/implement_class.h"
#include "rules/implement_interface_class.h"
#include "utils/init.h"

namespace SL = SURELOG;

namespace fs = std::filesystem;

namespace {

auto BasePath() -> fs::path {
  return fs::current_path() / ".." / ".." / "tests" / "ClassChecks";
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
    return vec[0].second;
  } catch (const std::exception& e) {
    std::cerr << "Compiler error: " << e.what() << '\n';
    return nullptr;
  }
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

TEST(ExtendClassTest, NoError) {
  const fs::path tests_path{BasePath() / "ExtendClass" / "NoError"};

  testCheckWithNoErrorsExpected(tests_path, CheckExtendClass);
}

TEST(ExtendClassTest, RaiseError) {
  const fs::path tests_path{BasePath() / "ExtendClass" / "RaiseError"};

  const std::unordered_set<SURELOG::ErrorDefinition::ErrorType> ignoreList{
      SURELOG::ErrorDefinition::COMP_UNDEFINED_BASE_CLASS};

  testCheckWithErrorsExpected(tests_path, verihogg_lint::LINT_EXTEND_CLASS,
                              ignoreList, CheckExtendClass);
}

TEST(DuplicateClassTest, NoError) {
  const fs::path tests_path{BasePath() / "DuplicateClass" / "NoError"};

  testCheckWithNoErrorsExpected(tests_path, CheckDuplicateClass);
}

TEST(DuplicateClassTest, RaiseError) {
  const fs::path tests_path{BasePath() / "DuplicateClass" / "RaiseError"};

  const std::unordered_set<SURELOG::ErrorDefinition::ErrorType> ignoreList{};

  testCheckWithErrorsExpected(tests_path, verihogg_lint::LINT_DUPLICATE_CLASS,
                              ignoreList, CheckDuplicateClass);
}

TEST(DuplicateConstructorTest, NoError) {
  const fs::path tests_path{BasePath() / "DuplicateConstructor" / "NoError"};

  testCheckWithNoErrorsExpected(tests_path, CheckDuplicateConstructor);
}

TEST(DuplicateConstructorTest, RaiseError) {
  const fs::path tests_path{BasePath() / "DuplicateConstructor" / "RaiseError"};

  const std::unordered_set<SURELOG::ErrorDefinition::ErrorType> ignoreList{};

  testCheckWithErrorsExpected(tests_path,
                              verihogg_lint::LINT_DUPLICATE_CONSTRUCTOR,
                              ignoreList, CheckDuplicateConstructor);
}

TEST(ExternConstraintUndeclaredTest, NoError) {
  const fs::path tests_path{BasePath() / "ExternConstraintUndeclared" /
                            "NoError"};

  testCheckWithNoErrorsExpected(tests_path, CheckExternConstraintUndeclared);
}

TEST(ExternConstraintUndeclaredTest, RaiseError) {
  const fs::path tests_path{BasePath() / "ExternConstraintUndeclared" /
                            "RaiseError"};

  const std::unordered_set<SURELOG::ErrorDefinition::ErrorType> ignoreList{
      SURELOG::ErrorDefinition::PA_SYNTAX_ERROR};

  testCheckWithErrorsExpected(tests_path,
                              verihogg_lint::LINT_EXTERN_CONSTRAINT_UNDECLARED,
                              ignoreList, CheckExternConstraintUndeclared);
}

TEST(ExternFunctionUndeclaredTest, NoError) {
  const fs::path tests_path{BasePath() / "ExternFunctionUndeclared" /
                            "NoError"};

  testCheckWithNoErrorsExpected(tests_path, CheckExternFunctionUndeclared);
}

TEST(ExternFunctionUndeclaredTest, RaiseError) {
  const fs::path tests_path{BasePath() / "ExternFunctionUndeclared" /
                            "RaiseError"};

  const std::unordered_set<SURELOG::ErrorDefinition::ErrorType> ignoreList{
      SURELOG::ErrorDefinition::PA_SYNTAX_ERROR};

  testCheckWithErrorsExpected(tests_path,
                              verihogg_lint::LINT_EXTERN_FUNCTION_UNDECLARED,
                              ignoreList, CheckExternFunctionUndeclared);
}

TEST(ExternTaskUndeclaredTest, NoError) {
  const fs::path tests_path{BasePath() / "ExternTaskUndeclared" / "NoError"};

  testCheckWithNoErrorsExpected(tests_path, CheckExternTaskUndeclared);
}

TEST(ExternTaskUndeclaredTest, RaiseError) {
  const fs::path tests_path{BasePath() / "ExternTaskUndeclared" / "RaiseError"};

  const std::unordered_set<SURELOG::ErrorDefinition::ErrorType> ignoreList{
      SURELOG::ErrorDefinition::PA_SYNTAX_ERROR};

  testCheckWithErrorsExpected(tests_path,
                              verihogg_lint::LINT_EXTERN_TASK_UNDECLARED,
                              ignoreList, CheckExternTaskUndeclared);
}

TEST(ExtendInterfaceClassTest, NoError) {
  const fs::path tests_path{BasePath() / "ExtendInterfaceClass" / "NoError"};

  testCheckWithNoErrorsExpected(tests_path, CheckExtendInterfaceClass);
}

TEST(ExtendInterfaceClassTest, RaiseError) {
  const fs::path tests_path{BasePath() / "ExtendInterfaceClass" / "RaiseError"};

  const std::unordered_set<SURELOG::ErrorDefinition::ErrorType> ignoreList{
      SURELOG::ErrorDefinition::COMP_UNDEFINED_BASE_CLASS,
      SURELOG::ErrorDefinition::PA_SYNTAX_ERROR};

  testCheckWithErrorsExpected(tests_path,
                              verihogg_lint::LINT_EXTEND_INTERFACE_CLASS,
                              ignoreList, CheckExtendInterfaceClass);
}

TEST(ImplementClassTest, NoError) {
  const fs::path tests_path{BasePath() / "ImplementClass" / "NoError"};

  testCheckWithNoErrorsExpected(tests_path, CheckImplementClass);
}

TEST(ImplementClassTest, RaiseError) {
  const fs::path tests_path{BasePath() / "ImplementClass" / "RaiseError"};

  const std::unordered_set<SURELOG::ErrorDefinition::ErrorType> ignoreList{
      verihogg_lint::LINT_EXTERN_FUNCTION_UNDECLARED};

  testCheckWithErrorsExpected(tests_path, verihogg_lint::LINT_IMPLEMENT_CLASS,
                              ignoreList, CheckImplementClass);
}

TEST(ImplementInterfaceClassTest, NoError) {
  const fs::path tests_path{BasePath() / "ImplementInterfaceClass" / "NoError"};

  testCheckWithNoErrorsExpected(tests_path, CheckImplementInterfaceClass);
}

TEST(ImplementInterfaceClassTest, RaiseError) {
  const fs::path tests_path{BasePath() / "ImplementInterfaceClass" /
                            "RaiseError"};

  const std::unordered_set<SURELOG::ErrorDefinition::ErrorType> ignoreList{};

  testCheckWithErrorsExpected(tests_path,
                              verihogg_lint::LINT_IMPLEMENT_INTERFACE_CLASS,
                              ignoreList, CheckImplementInterfaceClass);
}

TEST(ImplementCircularInheritanceTest, NoError) {
  const fs::path tests_path{BasePath() / "CircularInheritance" / "NoError"};

  testCheckWithNoErrorsExpected(tests_path, CheckCircularInheritance);
}

TEST(ImplementCircularInheritanceTest, RaiseError) {
  const fs::path tests_path{BasePath() / "CircularInheritance" / "RaiseError"};

  const std::unordered_set<SURELOG::ErrorDefinition::ErrorType> ignoreList{};

  testCheckWithErrorsExpected(tests_path,
                              verihogg_lint::LINT_CIRCULAR_INHERITANCE,
                              ignoreList, CheckCircularInheritance);
}

}  // namespace

auto main(int argc, char** argv) -> int {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
