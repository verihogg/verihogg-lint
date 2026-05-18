#include <Surelog/API/Surelog.h>
#include <Surelog/CommandLine/CommandLineParser.h>
#include <Surelog/Design/Design.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/ErrorReporting/ErrorDefinition.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <functional>

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
#include "utils.h"

namespace fs = std::filesystem;

namespace {
auto BasePath() -> const fs::path {
  return fs::current_path() / ".." / ".." / "tests" / "ClassChecks";
}

TEST(ExtendClassTest, NoError) {
  const fs::path tests_path{BasePath() / "ExtendClass" / "NoError"};

  global::CheckWithNoErrorsExpected(tests_path, CheckExtendClass);
}

TEST(ExtendClassTest, RaiseError) {
  const fs::path tests_path{BasePath() / "ExtendClass" / "RaiseError"};

  const std::unordered_set<SURELOG::ErrorDefinition::ErrorType> ignoreList{
      SURELOG::ErrorDefinition::COMP_UNDEFINED_BASE_CLASS};

  global::CheckWithErrorsExpected(tests_path, verihogg_lint::LINT_EXTEND_CLASS,
                                  ignoreList, CheckExtendClass);
}

TEST(DuplicateClassTest, NoError) {
  const fs::path tests_path{BasePath() / "DuplicateClass" / "NoError"};

  global::CheckWithNoErrorsExpected(tests_path, CheckDuplicateClass);
}

TEST(DuplicateClassTest, RaiseError) {
  const fs::path tests_path{BasePath() / "DuplicateClass" / "RaiseError"};

  const std::unordered_set<SURELOG::ErrorDefinition::ErrorType> ignoreList{};

  global::CheckWithErrorsExpected(tests_path,
                                  verihogg_lint::LINT_DUPLICATE_CLASS,
                                  ignoreList, CheckDuplicateClass);
}

TEST(DuplicateConstructorTest, NoError) {
  const fs::path tests_path{BasePath() / "DuplicateConstructor" / "NoError"};

  global::CheckWithNoErrorsExpected(tests_path, CheckDuplicateConstructor);
}

TEST(DuplicateConstructorTest, RaiseError) {
  const fs::path tests_path{BasePath() / "DuplicateConstructor" / "RaiseError"};

  const std::unordered_set<SURELOG::ErrorDefinition::ErrorType> ignoreList{};

  global::CheckWithErrorsExpected(tests_path,
                                  verihogg_lint::LINT_DUPLICATE_CONSTRUCTOR,
                                  ignoreList, CheckDuplicateConstructor);
}

TEST(ExternConstraintUndeclaredTest, NoError) {
  const fs::path tests_path{BasePath() / "ExternConstraintUndeclared" /
                            "NoError"};

  global::CheckWithNoErrorsExpected(tests_path,
                                    CheckExternConstraintUndeclared);
}

TEST(ExternConstraintUndeclaredTest, RaiseError) {
  const fs::path tests_path{BasePath() / "ExternConstraintUndeclared" /
                            "RaiseError"};

  const std::unordered_set<SURELOG::ErrorDefinition::ErrorType> ignoreList{
      SURELOG::ErrorDefinition::PA_SYNTAX_ERROR};

  global::CheckWithErrorsExpected(
      tests_path, verihogg_lint::LINT_EXTERN_CONSTRAINT_UNDECLARED, ignoreList,
      CheckExternConstraintUndeclared);
}

TEST(ExternFunctionUndeclaredTest, NoError) {
  const fs::path tests_path{BasePath() / "ExternFunctionUndeclared" /
                            "NoError"};

  global::CheckWithNoErrorsExpected(tests_path, CheckExternFunctionUndeclared);
}

TEST(ExternFunctionUndeclaredTest, RaiseError) {
  const fs::path tests_path{BasePath() / "ExternFunctionUndeclared" /
                            "RaiseError"};

  const std::unordered_set<SURELOG::ErrorDefinition::ErrorType> ignoreList{
      SURELOG::ErrorDefinition::PA_SYNTAX_ERROR};

  global::CheckWithErrorsExpected(
      tests_path, verihogg_lint::LINT_EXTERN_FUNCTION_UNDECLARED, ignoreList,
      CheckExternFunctionUndeclared);
}

TEST(ExternTaskUndeclaredTest, NoError) {
  const fs::path tests_path{BasePath() / "ExternTaskUndeclared" / "NoError"};

  global::CheckWithNoErrorsExpected(tests_path, CheckExternTaskUndeclared);
}

TEST(ExternTaskUndeclaredTest, RaiseError) {
  const fs::path tests_path{BasePath() / "ExternTaskUndeclared" / "RaiseError"};

  const std::unordered_set<SURELOG::ErrorDefinition::ErrorType> ignoreList{
      SURELOG::ErrorDefinition::PA_SYNTAX_ERROR};

  global::CheckWithErrorsExpected(tests_path,
                                  verihogg_lint::LINT_EXTERN_TASK_UNDECLARED,
                                  ignoreList, CheckExternTaskUndeclared);
}

TEST(ExtendInterfaceClassTest, NoError) {
  const fs::path tests_path{BasePath() / "ExtendInterfaceClass" / "NoError"};

  global::CheckWithNoErrorsExpected(tests_path, CheckExtendInterfaceClass);
}

TEST(ExtendInterfaceClassTest, RaiseError) {
  const fs::path tests_path{BasePath() / "ExtendInterfaceClass" / "RaiseError"};

  const std::unordered_set<SURELOG::ErrorDefinition::ErrorType> ignoreList{
      SURELOG::ErrorDefinition::COMP_UNDEFINED_BASE_CLASS,
      SURELOG::ErrorDefinition::PA_SYNTAX_ERROR};

  global::CheckWithErrorsExpected(tests_path,
                                  verihogg_lint::LINT_EXTEND_INTERFACE_CLASS,
                                  ignoreList, CheckExtendInterfaceClass);
}

TEST(ImplementClassTest, NoError) {
  const fs::path tests_path{BasePath() / "ImplementClass" / "NoError"};

  global::CheckWithNoErrorsExpected(tests_path, CheckImplementClass);
}

TEST(ImplementClassTest, RaiseError) {
  const fs::path tests_path{BasePath() / "ImplementClass" / "RaiseError"};

  const std::unordered_set<SURELOG::ErrorDefinition::ErrorType> ignoreList{
      verihogg_lint::LINT_EXTERN_FUNCTION_UNDECLARED};

  global::CheckWithErrorsExpected(tests_path,
                                  verihogg_lint::LINT_IMPLEMENT_CLASS,
                                  ignoreList, CheckImplementClass);
}

TEST(ImplementInterfaceClassTest, NoError) {
  const fs::path tests_path{BasePath() / "ImplementInterfaceClass" / "NoError"};

  global::CheckWithNoErrorsExpected(tests_path, CheckImplementInterfaceClass);
}

TEST(ImplementInterfaceClassTest, RaiseError) {
  const fs::path tests_path{BasePath() / "ImplementInterfaceClass" /
                            "RaiseError"};

  const std::unordered_set<SURELOG::ErrorDefinition::ErrorType> ignoreList{};

  global::CheckWithErrorsExpected(tests_path,
                                  verihogg_lint::LINT_IMPLEMENT_INTERFACE_CLASS,
                                  ignoreList, CheckImplementInterfaceClass);
}

TEST(ImplementCircularInheritanceTest, NoError) {
  const fs::path tests_path{BasePath() / "CircularInheritance" / "NoError"};

  global::CheckWithNoErrorsExpected(tests_path, CheckCircularInheritance);
}

TEST(ImplementCircularInheritanceTest, RaiseError) {
  const fs::path tests_path{BasePath() / "CircularInheritance" / "RaiseError"};

  const std::unordered_set<SURELOG::ErrorDefinition::ErrorType> ignoreList{};

  global::CheckWithErrorsExpected(tests_path,
                                  verihogg_lint::LINT_CIRCULAR_INHERITANCE,
                                  ignoreList, CheckCircularInheritance);
}

}  // namespace
auto main(int argc, char** argv) -> int {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
