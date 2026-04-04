#include "main/rule_dispatcher.h"

#include <Surelog/Common/FileSystem.h>
#include <Surelog/Design/Design.h>
#include <Surelog/Design/FileContent.h>
#include <uhdm/vpi_user.h>
#include <yaml-cpp/node/convert.h>      // NOLINT(misc-include-cleaner)
#include <yaml-cpp/node/detail/impl.h>  // NOLINT(misc-include-cleaner)
#include <yaml-cpp/node/emit.h>
#include <yaml-cpp/node/node.h>
#include <yaml-cpp/node/parse.h>
#include <yaml-cpp/null.h>
#include <yaml-cpp/parser.h>

#include <array>
#include <filesystem>
#include <functional>
#include <iostream>
#include <string>
#include <string_view>

#include "rules/assertion_statement_atribute_instance.h"
#include "rules/assignment_pattern.h"
#include "rules/assignment_pattern_context.h"
#include "rules/circular_inheritance.h"
#include "rules/class_variable_lifetime.h"
#include "rules/concatenation_multiplier.h"
#include "rules/covergroup_expression.h"
#include "rules/coverpoint_expression_type.h"
#include "rules/dpi_decl_string.h"
#include "rules/duplicate_class.h"
#include "rules/duplicate_constructor.h"
#include "rules/empty_assignment_pattern.h"
#include "rules/exponent_format_time_value.h"
#include "rules/extend_class.h"
#include "rules/extend_interface_class.h"
#include "rules/extern_constraint_undeclared.h"
#include "rules/extern_function_undeclared.h"
#include "rules/extern_task_undeclared.h"
#include "rules/fatal_rule.h"
#include "rules/foreach_loop_condition.h"
#include "rules/function_implemention_scope.h"
#include "rules/hierarchical_interface_identifier.h"
#include "rules/implement_class.h"
#include "rules/implement_interface_class.h"
#include "rules/implicit_data_type.h"
#include "rules/inside_operator.h"
#include "rules/inside_operator_range.h"
#include "rules/missing_for_loop_condition.h"
#include "rules/missing_for_loop_initialization.h"
#include "rules/missing_for_loop_step.h"
#include "rules/missing_function_implementation.h"
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
#include "yaml-cpp/node/node.h"
#include "yaml-cpp/null.h"

namespace SL = SURELOG;
constexpr const char* DefaultConfigFileName = ".verihogg-lint";

struct Rule {
  const std::string_view name;
  bool enabled = true;
  const std::function<void(const SL::FileContent*, SL::ErrorContainer*,
                           SL::SymbolTable*)>
      check;
};

struct GlobalRule {
  const std::string_view name;
  bool enabled = true;
  const std::function<void(SL::Design*, SL::ErrorContainer*, SL::SymbolTable*)>
      check;
};
namespace {
std::array kAllRules = std::to_array<Rule>({
    // clang-format off
    {.name = "RepetitionInSequence", .check = CheckRepetitionInSequence},
    {.name = "PrototypeReturnDataType", .check = CheckPrototypeReturnDataType},
    {.name = "ParameterDynamicArray", .check = CheckParameterDynamicArray},
    {.name = "ImplicitDataTypeInDeclaration", .check = CheckImplicitDataTypeInDeclaration},
    {.name = "HierarchicalInterfaceIdentifier", .check = CheckHierarchicalInterfaceIdentifier},
    {.name = "DpiDeclarationString", .check = CheckDpiDeclarationString},
    {.name = "ClassVariableLifetime", .check = CheckClassVariableLifetime},
    {.name = "CoverpointExpressionType", .check = CheckCoverpointExpressionType},
    {.name = "CovergroupExpression", .check = CheckCovergroupExpression},
    {.name = "ConcatenationMultiplier", .check = CheckConcatenationMultiplier},
    {.name = "ParameterOverride", .check = CheckParameterOverride},
    {.name = "MultipleDotStarConnections", .check = CheckMultipleDotStarConnections},
    {.name = "SelectInEventControl", .check = CheckSelectInEventControl},
    {.name = "EmptyAssignmentPattern", .check = CheckEmptyAssignmentPattern},
    {.name = "MissingForLoopInitialization", .check = CheckMissingForLoopInitialization},
    {.name = "MissingForLoopCondition", .check = CheckMissingForLoopCondition},
    {.name = "MissingForLoopStep", .check = CheckMissingForLoopStep},
    {.name = "ForeachLoopCondition", .check = CheckForeachLoopCondition},
    {.name = "SelectInWeight", .check = CheckSelectInWeight},
    {.name = "AssignmentPattern", .check = CheckAssignmentPattern},
    {.name = "AssignmentPatternContext", .check = CheckAssignmentPatternContext},
    {.name = "ScalarAssignmentPattern", .check = CheckScalarAssignmentPattern},
    {.name = "TargetUnpackedArrayConcatenation", .check = CheckTargetUnpackedArrayConcatenation},
    {.name = "InsideOperator", .check = CheckInsideOperator},
    {.name = "InsideOperatorRange", .check = CheckInsideOperatorRange},
    {.name = "TypeCasting", .check = CheckTypeCasting},
    {.name = "TimeValue", .check = CheckTimeValue},
    {.name = "MultipleBins", .check = CheckMultipleBins},
    {.name = "AssertionstatementAttributeInstance", .check = CheckAssertionStatementAttributeInstance},
    {.name = "SystemFunctionArguments", .check = CheckSystemFunctionArguments},
    {.name = "WildcardOperator", .check = CheckWildcardOperators},
    {.name = "ExponentFormatTimeValue", .check = CheckExponentFormatTimeValue},
    {.name = "ExtendClass", .check = CheckExtendClass},
    {.name = "DuplicateConstructor", .check = CheckDuplicateConstructor},
    {.name = "DuplicateClass", .check = CheckDuplicateClass},
    {.name = "ExternConstraintUndeclared", .check = CheckExternConstraintUndeclared},
    {.name = "ExternFunctionUndeclared", .check = CheckExternFunctionUndeclared},
    {.name = "ExternTaskUndeclared", .check = CheckExternTaskUndeclared},
    {.name = "ExtendInterfaceClass", .check = CheckExtendInterfaceClass},
    {.name = "ImplementClass", .check = CheckImplementClass},
    {.name = "ImplementInterfaceClass", .check = CheckImplementInterfaceClass},
    {.name = "CircularInheritance", .check = CheckCircularInheritance},
    // clang-format on
});

std::array kGlobalRules = std::to_array<GlobalRule>({
    // clang-format off
    {.name = "NofParameterOverrides", .check = CheckNofParameterOverrides},
    {.name = "MissingFunctionImplementation", .check = CheckMissingFunctionImplementation},
    {.name = "FuctionImplementationScope", .check = CheckFuncImplScope},
    // clang-format on
});

void RunAllRules(const SL::FileContent* fileContent, SL::ErrorContainer* errors,
                 SL::SymbolTable* symbols) {
  for (const auto& rule : kAllRules) {
    if (!rule.enabled) {
      continue;
    }
    rule.check(fileContent, errors, symbols);
  }
}

auto GetYamlConfig() -> YAML::Node {
  const std::filesystem::path configPath = DefaultConfigFileName;
  std::filesystem::path currentDir = std::filesystem::current_path();

  while (!std::filesystem::exists(currentDir / configPath)) {
    if (currentDir.parent_path() == currentDir) {
      std::cerr << "No config file" << "\n";
      return YAML::Node{};
    }
    currentDir = currentDir.parent_path();
  }

  return YAML::LoadFile(currentDir / configPath);
}

template <typename RuleType>
void FilterSpecificRule(RuleType& rule, const YAML::Node& tree) {
  const YAML::Node node = tree[rule.name];
  if (node) {
    const auto value = node.as<std::string>();

    if (value == "yes" || value == "true") {
      rule.enabled = true;
    } else if (value == "false" || value == "no") {
      rule.enabled = false;
    } else {
      std::cerr << "Expected boolean, got: "
                << std::string(value.begin(), value.end()) << "\n";
    }
  }
}

void FilterRules() {
  const auto yaml = GetYamlConfig();
  if (yaml.IsNull()) {
    return;
  }

  for (auto& rule : kAllRules) {
    FilterSpecificRule(rule, yaml);
  }
  for (auto& rule : kGlobalRules) {
    FilterSpecificRule(rule, yaml);
  }
}
}  // namespace

void RunAllRulesOnDesign(SL::Design* design, const vpiHandle& uhdmDesign,
                         SL::ErrorContainer* errors, SL::SymbolTable* symbols) {
  if (design == nullptr) {
    return;
  }

  FilterRules();

  for (auto& [name, fileContent] : design->getAllFileContents()) {
    if (fileContent == nullptr) {
      continue;
    }

    RunAllRules(fileContent, errors, symbols);
    FatalListener listener(errors, symbols);
    listener.Listen(uhdmDesign);
  }

  for (const auto& rule : kGlobalRules) {
    if (!rule.enabled) {
      continue;
    }
    rule.check(design, errors, symbols);
  }
}
