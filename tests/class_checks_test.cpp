#include <gtest/gtest.h>
#include <rules/duplicate_class.h>
#include <rules/duplicate_constructor.h>
#include <rules/extend_class.h>
#include <rules/extern_constraint_undeclared.h>
#include <rules/extern_function_undeclared.h>
#include <rules/extern_task_undeclared.h>
#include <utils/init.h>

#include <filesystem>
#include <functional>
#include <memory>
#include <set>

#include "Surelog/API/Surelog.h"
#include "Surelog/CommandLine/CommandLineParser.h"
#include "Surelog/Design/Design.h"
#include "Surelog/Design/FileContent.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"

namespace fs = std::filesystem;

const fs::path base_path =
    fs::current_path() / ".." / ".." / "tests" / "ClassChecks";

namespace {

FileContent* getFileContentFromPath(fs::path path, ErrorContainer* errors,
                                    SymbolTable* symbols) {
  const auto clp =
      std::make_unique<CommandLineParser>(errors, symbols, false, false);
  initCommandLineParser(clp.get());

  const std::string path_str = path.string();
  const char* argv[2] = {"", path_str.c_str()};
  if (!clp->parseCommandLine(2, argv)) {
    std::cerr << "Can't parse command line" << std::endl;
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
    const fs::path tests_path,
    std::function<void(const FileContent*, ErrorContainer*, SymbolTable*)>
        check_func) {
  std::vector<fs::path> paths(fs::directory_iterator{tests_path},
                              fs::directory_iterator{});

  for (auto& file_path : paths) {
    std::cout << file_path << std::endl;
    auto symbols = std::make_unique<SymbolTable>();
    auto errors = std::make_unique<ErrorContainer>(symbols.get());

    const FileContent* fC =
        getFileContentFromPath(file_path, errors.get(), symbols.get());
    check_func(fC, errors.get(), symbols.get());
    errors->printMessages();
    ASSERT_EQ(errors.get()->getErrors().size(), 0);
  }
}

void testCheckWithErrorsExpected(
    const fs::path tests_path, ErrorDefinition::ErrorType errorIdExpected,
    std::set<ErrorDefinition::ErrorType> ignoreList,
    std::function<void(const FileContent*, ErrorContainer*, SymbolTable*)>
        check_func) {
  std::vector<fs::path> paths(fs::directory_iterator{tests_path},
                              fs::directory_iterator{});

  for (auto& file_path : paths) {
    std::cout << file_path << std::endl;
    auto symbols = std::make_unique<SymbolTable>();
    auto errors = std::make_unique<ErrorContainer>(symbols.get());

    const FileContent* fC =
        getFileContentFromPath(file_path, errors.get(), symbols.get());
    check_func(fC, errors.get(), symbols.get());
    errors->printMessages();

    auto errorVector = errors.get()->getErrors();
    ASSERT_NE(errorVector.size(), 0);
    bool hasError = false;
    for (auto& error : errorVector) {
      ErrorDefinition::ErrorType type = error.getType();
      if (ignoreList.count(type) > 0) continue;
      hasError = true;
      ASSERT_EQ(type, errorIdExpected);
    }
    ASSERT_EQ(hasError, true);
  }
}

TEST(ExtendClassTest, NoError) {
  const fs::path tests_path{base_path / "ExtendClass" / "NoError"};

  testCheckWithNoErrorsExpected(tests_path, checkExtendClass);
}

TEST(ExtendClassTest, RaiseError) {
  const fs::path tests_path{base_path / "ExtendClass" / "RaiseError"};

  std::set<ErrorDefinition::ErrorType> ignoreList{
      ErrorDefinition::COMP_UNDEFINED_BASE_CLASS};

  testCheckWithErrorsExpected(tests_path, ErrorDefinition::LINT_EXTEND_CLASS,
                              ignoreList, checkExtendClass);
}

TEST(DuplicateClassTest, NoError) {
  const fs::path tests_path{base_path / "DuplicateClass" / "NoError"};

  testCheckWithNoErrorsExpected(tests_path, checkDuplicateClass);
}

TEST(DuplicateClassTest, RaiseError) {
  const fs::path tests_path{base_path / "DuplicateClass" / "RaiseError"};

  std::set<ErrorDefinition::ErrorType> ignoreList{};

  testCheckWithErrorsExpected(tests_path, ErrorDefinition::LINT_DUPLICATE_CLASS,
                              ignoreList, checkDuplicateClass);
}

TEST(DuplicateConstructorTest, NoError) {
  const fs::path tests_path{base_path / "DuplicateConstructor" / "NoError"};

  testCheckWithNoErrorsExpected(tests_path, checkDuplicateConstructor);
}

TEST(DuplicateConstructorTest, RaiseError) {
  const fs::path tests_path{base_path / "DuplicateConstructor" / "RaiseError"};

  std::set<ErrorDefinition::ErrorType> ignoreList{};

  testCheckWithErrorsExpected(tests_path,
                              ErrorDefinition::LINT_DUPLICATE_CONSTRUCTOR,
                              ignoreList, checkDuplicateConstructor);
}

TEST(ExternConstraintUndeclaredTest, NoError) {
  const fs::path tests_path{base_path / "ExternContraintUndeclared" /
                            "NoError"};

  testCheckWithNoErrorsExpected(tests_path, checkExternConstraintUndeclared);
}

TEST(ExternConstraintUndeclaredTest, RaiseError) {
  const fs::path tests_path{base_path / "ExternContraintUndeclared" /
                            "RaiseError"};

  std::set<ErrorDefinition::ErrorType> ignoreList{};

  testCheckWithErrorsExpected(
      tests_path, ErrorDefinition::LINT_EXTERN_CONSTRAINT_UNDECLARED,
      ignoreList, checkExternConstraintUndeclared);
}

TEST(ExternFunctionUndeclaredTest, NoError) {
  const fs::path tests_path{base_path / "ExternFunctionUndeclared" / "NoError"};

  testCheckWithNoErrorsExpected(tests_path, checkExternFunctionUndeclared);
}

TEST(ExternFunctionUndeclaredTest, RaiseError) {
  const fs::path tests_path{base_path / "ExternFunctionUndeclared" /
                            "RaiseError"};

  std::set<ErrorDefinition::ErrorType> ignoreList{
      ErrorDefinition::PA_SYNTAX_ERROR};

  testCheckWithErrorsExpected(tests_path,
                              ErrorDefinition::LINT_EXTERN_FUNCTION_UNDECLARED,
                              ignoreList, checkExternFunctionUndeclared);
}

TEST(ExternTaskUndeclaredTest, NoError) {
  const fs::path tests_path{base_path / "ExternTaskUndeclared" / "NoError"};

  testCheckWithNoErrorsExpected(tests_path, checkExternTaskUndeclared);
}

TEST(ExternTaskUndeclaredTest, RaiseError) {
  const fs::path tests_path{base_path / "ExternTaskUndeclared" / "RaiseError"};

  std::set<ErrorDefinition::ErrorType> ignoreList{
      ErrorDefinition::PA_SYNTAX_ERROR};

  testCheckWithErrorsExpected(tests_path,
                              ErrorDefinition::LINT_EXTERN_TASK_UNDECLARED,
                              ignoreList, checkExternTaskUndeclared);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

}  // namespace
