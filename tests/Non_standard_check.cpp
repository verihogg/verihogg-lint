#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <filesystem>
#include <functional>
#include <iostream>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "Surelog/API/Surelog.h"
#include "Surelog/CommandLine/CommandLineParser.h"
#include "Surelog/Design/Design.h"
#include "Surelog/Design/FileContent.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"
#include "main/lint_rules.h"
#include "rules/assertion_statement_atribute_instance.h"
#include "rules/assignment_pattern.h"
#include "rules/assignment_pattern_context.h"
#include "rules/class_variable_lifetime.h"
#include "rules/concatenation_multiplier.h"
#include "rules/covergroup_expression.h"
#include "rules/coverpoint_expression_type.h"
#include "rules/dpi_decl_string.h"
#include "rules/empty_assignment_pattern.h"
#include "rules/event_control_expression.h"
#include "rules/exponent_format_time_value.h"
#include "rules/foreach_loop_condition.h"
#include "rules/function_implementation_return_type.h"
#include "rules/function_implemention_scope.h"
#include "rules/hierarchical_interface_identifier.h"
#include "rules/implicit_data_type.h"
#include "rules/inside_operator.h"
#include "rules/inside_operator_range.h"
#include "rules/method_override_argument_name.h"
#include "rules/missing_for_loop_condition.h"
#include "rules/missing_for_loop_initialization.h"
#include "rules/missing_for_loop_step.h"
#include "rules/missing_function_implementation.h"
#include "rules/modport_import_export_port.h"
#include "rules/multiple_bins.h"
#include "rules/multiple_dot_star_connection.h"
#include "rules/nof_parameter_override.h"
#include "rules/parameter_dynamic_array.h"
#include "rules/parameter_override.h"
#include "rules/prototype_return_data_type.h"
#include "rules/repeat_in_sequence.h"
#include "rules/scalar_assignment_pattern.h"
#include "rules/select_in_event_control.h"
#include "rules/select_in_weight.h"
#include "rules/system_function_arguments.h"
#include "rules/target_unpacked_array_concatenation.h"
#include "rules/time_value.h"
#include "rules/type_casting.h"
#include "rules/wildcard_operator.h"

namespace SL = SURELOG;
namespace fs = std::filesystem;

#ifndef VERIHOGG_TEST_NON_STANDARD_ROOT
#error "VERIHOGG_TEST_NON_STANDARD_ROOT must be defined by CMake"
#endif

namespace {

void LogErrorsIfAny(const fs::path& file_path, SL::ErrorContainer* errors) {
  const auto& vec = errors->getErrors();
  if (!vec.empty()) {
    std::cerr << "[ERRORS] File: " << file_path << '\n';
    errors->printMessages();
  }
}

auto BasePath() -> const fs::path& {
  static const fs::path base_path{VERIHOGG_TEST_NON_STANDARD_ROOT};
  return base_path;
}

using CheckFunc = void (*)(const SL::FileContent*, SL::ErrorContainer*,
                           SL::SymbolTable*);
using CheckFuncGlobal = void (*)(SL::Design*, SL::ErrorContainer*,
                                 SL::SymbolTable*);

struct RuleSpec {
  std::string folder;
  SURELOG::ErrorDefinition::ErrorType expected_error;
  CheckFunc check;
  std::unordered_set<SURELOG::ErrorDefinition::ErrorType> ignore_errors;
};

struct RuleSpecGlobal {
  std::string folder;
  SURELOG::ErrorDefinition::ErrorType expected_error;
  CheckFuncGlobal check;
  std::unordered_set<SURELOG::ErrorDefinition::ErrorType> ignore_errors;
};

void PrintTo(const RuleSpec& spec, std::ostream* os) {
  *os << "RuleSpec{folder=" << spec.folder
      << ", expected_error=" << static_cast<int>(spec.expected_error) << "}";
}

void PrintTo(const RuleSpecGlobal& spec, std::ostream* os) {
  *os << "RuleSpecGlobal{folder=" << spec.folder
      << ", expected_error=" << static_cast<int>(spec.expected_error) << "}";
}

auto SanitizeTestName(std::string name) -> std::string {
  for (char& c : name) {
    const auto uc = static_cast<unsigned char>(c);
    if (!std::isalnum(uc) && c != '_') {
      c = '_';
    }
  }

  if (name.empty()) {
    return "Empty";
  }

  if (std::isdigit(static_cast<unsigned char>(name.front()))) {
    name.insert(name.begin(), '_');
  }

  return name;
}

auto CollectCaseFiles(const fs::path& directory) -> std::vector<fs::path> {
  std::vector<fs::path> files;
  if (!fs::exists(directory)) {
    return files;
  }

  for (const auto& entry : fs::directory_iterator(directory)) {
    if (!entry.is_regular_file()) {
      continue;
    }

    const auto& path = entry.path();
    const std::string filename = path.filename().string();

    if (path.extension() == ".sv" && filename.starts_with("case")) {
      files.push_back(path);
    }
  }

  std::ranges::sort(files);
  return files;
}

auto GetFileContentFromPath(const fs::path& path, SL::ErrorContainer* errors,
                            SL::SymbolTable* symbols) -> SL::FileContent* {
  auto clp = std::make_unique<SURELOG::CommandLineParser>(errors, symbols,
                                                          false, false);
  clp->noPython();
  clp->setParse(true);
  clp->setCompile(true);
  clp->setElaborate(false);
  clp->setwritePpOutput(true);
  clp->setCacheAllowed(false);
  clp->setFilterInfo();
  clp->setFilterNote();
  clp->setFilterWarning();

  const std::string path_str = path.string();
  std::array<const char*, 2> argv = {
      "verihogg-lint-test",
      path_str.c_str(),
  };

  if (!clp->parseCommandLine(static_cast<int>(argv.size()), argv.data())) {
    throw std::runtime_error("Can't parse command line: " + path_str);
  }

  auto compiler = start_compiler(clp.get());
  auto* design = get_design(compiler);
  if (design == nullptr) {
    throw std::runtime_error("Compiler error: design is null: " + path_str);
  }

  auto& file_contents = design->getAllFileContents();
  if (file_contents.empty()) {
    throw std::runtime_error("No FileContent: " + path_str);
  }

  auto it = file_contents.begin();
  if (it == file_contents.end() || it->second == nullptr) {
    throw std::runtime_error("No FileContent: " + path_str);
  }

  return it->second;
}

auto GetDesignFromPath(const fs::path& path, SL::ErrorContainer* errors,
                       SL::SymbolTable* symbols) -> SL::Design* {
  auto clp = std::make_unique<SURELOG::CommandLineParser>(errors, symbols,
                                                          false, false);
  clp->noPython();
  clp->setParse(true);
  clp->setCompile(true);
  clp->setElaborate(false);
  clp->setwritePpOutput(true);
  clp->setCacheAllowed(false);
  clp->setFilterInfo();
  clp->setFilterNote();
  clp->setFilterWarning();

  const std::string path_str = path.string();
  std::array<const char*, 2> argv = {
      "verihogg-lint-test",
      path_str.c_str(),
  };

  if (!clp->parseCommandLine(static_cast<int>(argv.size()), argv.data())) {
    throw std::runtime_error("Can't parse command line: " + path_str);
  }

  auto compiler = start_compiler(clp.get());
  auto* design = get_design(compiler);
  if (design == nullptr) {
    throw std::runtime_error("Compiler error: design is null: " + path_str);
  }

  return design;
}

void testCheckWithNoErrorsExpected(
    const fs::path& tests_path,
    const std::function<void(const SL::FileContent*, SL::ErrorContainer*,
                             SL::SymbolTable*)>& check_func) {
  ASSERT_TRUE(fs::exists(tests_path));

  for (const auto& file_path : CollectCaseFiles(tests_path)) {
    SCOPED_TRACE(file_path.string());

    auto symbols = std::make_unique<SURELOG::SymbolTable>();
    auto errors = std::make_unique<SURELOG::ErrorContainer>(symbols.get());

    const SL::FileContent* fC =
        GetFileContentFromPath(file_path, errors.get(), symbols.get());
    check_func(fC, errors.get(), symbols.get());

    const auto& errorVector = errors->getErrors();

    if (!errorVector.empty()) {
      std::cerr << "[UNEXPECTED ERROR] File: " << file_path << '\n';
      errors->printMessages();
    }

    ASSERT_EQ(errorVector.size(), 0U)
        << "Unexpected errors in file: " << file_path;
  }
}

void testCheckWithErrorsExpected(
    const fs::path& tests_path,
    SURELOG::ErrorDefinition::ErrorType errorIdExpected,
    const std::unordered_set<SURELOG::ErrorDefinition::ErrorType>& ignoreList,
    const std::function<void(const SL::FileContent*, SL::ErrorContainer*,
                             SL::SymbolTable*)>& check_func) {
  ASSERT_TRUE(fs::exists(tests_path));

  for (const auto& file_path : CollectCaseFiles(tests_path)) {
    SCOPED_TRACE(file_path.string());

    auto symbols = std::make_unique<SURELOG::SymbolTable>();
    auto errors = std::make_unique<SURELOG::ErrorContainer>(symbols.get());

    const SL::FileContent* fC =
        GetFileContentFromPath(file_path, errors.get(), symbols.get());
    check_func(fC, errors.get(), symbols.get());

    const auto& errorVector = errors->getErrors();

    if (errorVector.empty()) {
      std::cerr << "[MISSING ERROR] File: " << file_path << '\n';
    } else {
      LogErrorsIfAny(file_path, errors.get());
    }

    ASSERT_FALSE(errorVector.empty())
        << "Expected error not found in file: " << file_path;

    bool hasExpectedError = false;

    for (const auto& error : errorVector) {
      const auto type = error.getType();

      if (ignoreList.count(type) > 0) {
        continue;
      }

      if (type != errorIdExpected) {
        std::cerr << "[WRONG ERROR TYPE] File: " << file_path
                  << " Got: " << static_cast<int>(type)
                  << " Expected: " << static_cast<int>(errorIdExpected) << '\n';
      }

      ASSERT_EQ(type, errorIdExpected)
          << "Wrong error type in file: " << file_path;

      hasExpectedError = true;
    }

    ASSERT_TRUE(hasExpectedError)
        << "Expected error type not found in file: " << file_path;
  }
}

void testCheckWithNoErrorsExpectedGlobal(
    const fs::path& tests_path,
    const std::function<void(SL::Design*, SL::ErrorContainer*,
                             SL::SymbolTable*)>& check_func) {
  ASSERT_TRUE(fs::exists(tests_path));

  for (const auto& file_path : CollectCaseFiles(tests_path)) {
    SCOPED_TRACE(file_path.string());

    auto symbols = std::make_unique<SURELOG::SymbolTable>();
    auto errors = std::make_unique<SURELOG::ErrorContainer>(symbols.get());

    SL::Design* design =
        GetDesignFromPath(file_path, errors.get(), symbols.get());
    check_func(design, errors.get(), symbols.get());

    check_func(design, errors.get(), symbols.get());

    const auto& errorVector = errors->getErrors();

    if (!errorVector.empty()) {
      std::cerr << "[UNEXPECTED ERROR][GLOBAL] File: " << file_path << '\n';
      errors->printMessages();
    }

    ASSERT_EQ(errorVector.size(), 0U)
        << "Unexpected errors in file: " << file_path;
  }
}

void testCheckWithErrorsExpectedGlobal(
    const fs::path& tests_path,
    SURELOG::ErrorDefinition::ErrorType errorIdExpected,
    const std::unordered_set<SURELOG::ErrorDefinition::ErrorType>& ignoreList,
    const std::function<void(SL::Design*, SL::ErrorContainer*,
                             SL::SymbolTable*)>& check_func) {
  ASSERT_TRUE(fs::exists(tests_path));

  for (const auto& file_path : CollectCaseFiles(tests_path)) {
    SCOPED_TRACE(file_path.string());

    auto symbols = std::make_unique<SURELOG::SymbolTable>();
    auto errors = std::make_unique<SURELOG::ErrorContainer>(symbols.get());

    SL::Design* design =
        GetDesignFromPath(file_path, errors.get(), symbols.get());
    check_func(design, errors.get(), symbols.get());

    const auto& errorVector = errors->getErrors();

    if (errorVector.empty()) {
      std::cerr << "[MISSING ERROR][GLOBAL] File: " << file_path << '\n';
    } else {
      LogErrorsIfAny(file_path, errors.get());
    }

    ASSERT_FALSE(errorVector.empty())
        << "Expected error not found in file: " << file_path;

    bool hasExpectedError = false;

    for (const auto& error : errorVector) {
      const auto type = error.getType();

      if (ignoreList.count(type) > 0) {
        continue;
      }

      if (type != errorIdExpected) {
        std::cerr << "[WRONG ERROR TYPE][GLOBAL] File: " << file_path
                  << " Got: " << static_cast<int>(type)
                  << " Expected: " << static_cast<int>(errorIdExpected) << '\n';
      }

      ASSERT_EQ(type, errorIdExpected)
          << "Wrong error type in file: " << file_path;

      hasExpectedError = true;
    }

    ASSERT_TRUE(hasExpectedError)
        << "Expected error type not found in file: " << file_path;
  }
}

auto RuleSpecs() -> const std::vector<RuleSpec>& {
  static const std::vector<RuleSpec> specs = {
      {"ClassVariableLifetime",
       verihogg_lint::LINT_CLASS_VARIABLE_LIFETIME,
       CheckClassVariableLifetime,
       {}},
      {"DpiDeclarationString",
       verihogg_lint::LINT_DPI_DECLARATION_STRING,
       CheckDpiDeclarationString,
       {}},
      {"HierarchicalInterfaceIdentifier",
       verihogg_lint::LINT_HIERARCHICAL_INTERFACE_IDENTIFIER,
       CheckHierarchicalInterfaceIdentifier,
       {}},
      {"ImplicitDataTypeInDeclaration",
       verihogg_lint::LINT_IMPLICIT_DATA_TYPE,
       CheckImplicitDataTypeInDeclaration,
       {}},
      {"ParameterDynamicArray",
       verihogg_lint::LINT_PARAMETR_DYNAMIC_ARRAY,
       CheckParameterDynamicArray,
       {}},
      {"PrototypeReturnDataType",
       verihogg_lint::LINT_PROTOTYPE_RETURN_DATA_TYPE,
       CheckPrototypeReturnDataType,
       {}},
      {"RepetitionInSequence",
       verihogg_lint::LINT_REPETITION_IN_SEQUENCE,
       CheckRepetitionInSequence,
       {}},
      {"CoverpointExpressionType",
       verihogg_lint::LINT_COVERPOINT_EXPRESSION_TYPE,
       CheckCoverpointExpressionType,
       {}},
      {"CovergroupExpression",
       verihogg_lint::LINT_COVERGROUP_EXPRESSION,
       CheckCovergroupExpression,
       {}},
      {"ConcatenationMultiplier",
       verihogg_lint::LINT_CONCATENATION_MULTIPLIER,
       CheckConcatenationMultiplier,
       {}},
      {"ParameterOverride",
       verihogg_lint::LINT_PARAMETR_OVERRIDE,
       CheckParameterOverride,
       {}},
      {"MultipleDotStarConnections",
       verihogg_lint::LINT_MULTIPLE_DOT_STAR_CONNECTIONS,
       CheckMultipleDotStarConnections,
       {}},
      {"SelectInEventControl",
       verihogg_lint::LINT_SELECT_IN_EVENT_CONTROL,
       CheckSelectInEventControl,
       {}},
      {"EmptyAssignmentPattern",
       verihogg_lint::LINT_EMPTY_ASSIGNMENT_PATTERN,
       CheckEmptyAssignmentPattern,
       {}},
      {"MissingForLoopInitialization",
       verihogg_lint::LINT_MISSING_FOR_LOOP_INITIALIZATION,
       CheckMissingForLoopInitialization,
       {}},
      {"MissingForLoopCondition",
       verihogg_lint::LINT_MISSING_FOR_LOOP_CONDITION,
       CheckMissingForLoopCondition,
       {}},
      {"MissingForLoopStep",
       verihogg_lint::LINT_MISSING_FOR_LOOP_STEP,
       CheckMissingForLoopStep,
       {}},
      {"ForeachLoopCondition",
       verihogg_lint::LINT_FOREACH_LOOP_CONDITION,
       CheckForeachLoopCondition,
       {}},
      {"SelectInWeight",
       verihogg_lint::LINT_SELECT_IN_WEIGHT,
       CheckSelectInWeight,
       {}},
      {"AssignmentPattern",
       verihogg_lint::LINT_ASSIGNMENT_PATTERN,
       CheckAssignmentPattern,
       {}},
      {"AssignmentPatternContext",
       verihogg_lint::LINT_ASSIGNMENT_PATTERN_CONTEXT,
       CheckAssignmentPatternContext,
       {}},
      {"ScalarAssignmentPattern",
       verihogg_lint::LINT_SCALAR_ASSIGNMENT_PATTERN,
       CheckScalarAssignmentPattern,
       {}},
      {"TargetUnpackedArrayConcatenation",
       verihogg_lint::LINT_TARGET_UNPACKED_ARRAY_CONCATENATION,
       CheckTargetUnpackedArrayConcatenation,
       {}},
      {"InsideOperator",
       verihogg_lint::LINT_INSIDE_OPERATOR,
       CheckInsideOperator,
       {}},
      {"InsideOperatorRange",
       verihogg_lint::LINT_INSIDE_OPERATOR_RANGE,
       CheckInsideOperatorRange,
       {}},
      {"TypeCasting", verihogg_lint::LINT_TYPE_CASTING, CheckTypeCasting, {}},
      {"TimeValue", verihogg_lint::LINT_TIME_VALUE, CheckTimeValue, {}},
      {"MultipleBins",
       verihogg_lint::LINT_MULTIPLE_BINS,
       CheckMultipleBins,
       {}},
      {"AssertionStatementAttributeInstance",
       verihogg_lint::LINT_ASSERTION_STATEMENT_ATTRIBUTE_INSTANCE,
       CheckAssertionStatementAttributeInstance,
       {}},
      {"SystemFunctionArguments",
       verihogg_lint::LINT_SYSTEM_FUNCTION_ARGUMENTS,
       CheckSystemFunctionArguments,
       {}},
      {"WildcardEqualityOperator",
       verihogg_lint::LINT_WILDCARD_EQUALITY_OPERATOR,
       CheckWildcardOperators,
       {}},
      {"ExponentFormatTimeValue",
       verihogg_lint::LINT_EXPONENT_FORMAT_TIME_VALUE,
       CheckExponentFormatTimeValue,
       {}},
      {"ModportImportExport",
       verihogg_lint::LINT_MODPORT_IMPORT_EXPORT_PORT,
       CheckModportImportExportPort,
       {}},
      {"EventControlExpression",
       verihogg_lint::LINT_EVENT_CONTROL_EXPRESSION,
       CheckEventControlExpression,
       {}},
  };
  return specs;
}

auto GlobalRuleSpecs() -> const std::vector<RuleSpecGlobal>& {
  static const std::vector<RuleSpecGlobal> specs = {
      {"NofParameterOverrides",
       verihogg_lint::LINT_NOF_PARAMETER_OVERRIDE,
       CheckNofParameterOverrides,
       {}},
      {"MissingFunctionImplementation",
       verihogg_lint::LINT_MISSING_FUNCTION_IMPLEMENTATION,
       CheckMissingFunctionImplementation,
       {}},
      {"FunctionImplementationScope",
       verihogg_lint::LINT_FUNC_IMPL_SCOPE,
       CheckFuncImplScope,
       {}},
      {"TaskImplementationScope",
       verihogg_lint::LINT_TASK_IMPL_SCOPE,
       CheckFuncImplScope,
       {}},
      {"ConstraintImplementationScope",
       verihogg_lint::LINT_CONSTRAINT_IMPL_SCOPE,
       CheckFuncImplScope,
       {}},
      {"MethodOverrideArgumentName",
       verihogg_lint::LINT_METHOD_OVERRIDE_ARGUMENT_NAME,
       CheckMethodOverrideArgumentName,
       {}},
      {"FunctionImplementationReturnType",
       verihogg_lint::LINT_FUNCTION_IMPLEMENTATION_RETURN_TYPE,
       CheckFunctionImplementationReturnType,
       {}},
  };
  return specs;
}

class RuleTestFixture : public ::testing::TestWithParam<RuleSpec> {};
class GlobalRuleTestFixture : public ::testing::TestWithParam<RuleSpecGlobal> {
};

auto RuleParamName(const ::testing::TestParamInfo<RuleSpec>& info)
    -> std::string {
  return SanitizeTestName(info.param.folder);
}

auto GlobalRuleParamName(const ::testing::TestParamInfo<RuleSpecGlobal>& info)
    -> std::string {
  return SanitizeTestName(info.param.folder);
}

TEST_P(RuleTestFixture, NoError) {
  const auto& spec = GetParam();
  const fs::path tests_path{BasePath() / spec.folder / "NoError"};

  testCheckWithNoErrorsExpected(
      tests_path,
      [check = spec.check](
          const SL::FileContent* fC, SL::ErrorContainer* errors,
          SL::SymbolTable* symbols) { check(fC, errors, symbols); });
}

TEST_P(RuleTestFixture, RaiseError) {
  const auto& spec = GetParam();
  const fs::path tests_path{BasePath() / spec.folder / "RaiseError"};

  testCheckWithErrorsExpected(
      tests_path, spec.expected_error, spec.ignore_errors,
      [check = spec.check](
          const SL::FileContent* fC, SL::ErrorContainer* errors,
          SL::SymbolTable* symbols) { check(fC, errors, symbols); });
}

TEST_P(GlobalRuleTestFixture, NoError) {
  const auto& spec = GetParam();
  const fs::path tests_path{BasePath() / spec.folder / "NoError"};

  testCheckWithNoErrorsExpectedGlobal(
      tests_path,
      [check = spec.check](SL::Design* design, SL::ErrorContainer* errors,
                           SL::SymbolTable* symbols) {
        check(design, errors, symbols);
      });
}

TEST_P(GlobalRuleTestFixture, RaiseError) {
  const auto& spec = GetParam();
  const fs::path tests_path{BasePath() / spec.folder / "RaiseError"};

  testCheckWithErrorsExpectedGlobal(
      tests_path, spec.expected_error, spec.ignore_errors,
      [check = spec.check](SL::Design* design, SL::ErrorContainer* errors,
                           SL::SymbolTable* symbols) {
        check(design, errors, symbols);
      });
}

INSTANTIATE_TEST_SUITE_P(AllRuleTests, RuleTestFixture,
                         ::testing::ValuesIn(RuleSpecs()), RuleParamName);

INSTANTIATE_TEST_SUITE_P(AllGlobalRuleTests, GlobalRuleTestFixture,
                         ::testing::ValuesIn(GlobalRuleSpecs()),
                         GlobalRuleParamName);

}  // namespace

auto main(int argc, char** argv) -> int {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}