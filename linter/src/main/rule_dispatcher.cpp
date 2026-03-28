#include "main/rule_dispatcher.h"

#include <Surelog/Common/FileSystem.h>
#include <Surelog/Design/Design.h>
#include <Surelog/Design/FileContent.h>
#include <uhdm/vpi_user.h>

#include <array>
#include <functional>
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

namespace SL = SURELOG;

struct Rule {
  std::string_view name;
  bool enabled = true;
  std::function<void(const SL::FileContent*, SL::ErrorContainer*,
                     SL::SymbolTable*)>
      check;
};

struct GlobalRule {
  std::string_view name;
  bool enabled = true;
  std::function<void(SL::Design*, SL::ErrorContainer*, SL::SymbolTable*)> check;
};

static const std::array kAllRules = std::to_array<Rule>({
    {.name = "RepetitionInSequence",
     .enabled = true,
     .check = CheckRepetitionInSequence},
    {.name = "PrototypeReturnDataType",
     .enabled = true,
     .check = CheckPrototypeReturnDataType},
    {.name = "ParameterDynamicArray",
     .enabled = true,
     .check = CheckParameterDynamicArray},
    {.name = "ImplicitDataTypeInDeclaration",
     .enabled = true,
     .check = CheckImplicitDataTypeInDeclaration},
    {.name = "HierarchicalInterfaceIdentifier",
     .enabled = true,
     .check = CheckHierarchicalInterfaceIdentifier},
    {.name = "DpiDeclarationString",
     .enabled = true,
     .check = CheckDpiDeclarationString},
    {.name = "ClassVariableLifetime",
     .enabled = true,
     .check = CheckClassVariableLifetime},
    {.name = "CoverpointExpressionType",
     .enabled = true,
     .check = CheckCoverpointExpressionType},
    {.name = "CovergroupExpression",
     .enabled = true,
     .check = CheckCovergroupExpression},
    {.name = "ConcatenationMultiplier",
     .enabled = true,
     .check = CheckConcatenationMultiplier},
    {.name = "ParameterOverride",
     .enabled = true,
     .check = CheckParameterOverride},
    {.name = "MultipleDotStarConnections",
     .enabled = true,
     .check = CheckMultipleDotStarConnections},
    {.name = "SelectInEventControl",
     .enabled = true,
     .check = CheckSelectInEventControl},
    {.name = "EmptyAssignmentPattern",
     .enabled = true,
     .check = CheckEmptyAssignmentPattern},
    {.name = "MissingForLoopInitialization",
     .enabled = true,
     .check = CheckMissingForLoopInitialization},
    {.name = "MissingForLoopCondition",
     .enabled = true,
     .check = CheckMissingForLoopCondition},
    {.name = "MissingForLoopStep",
     .enabled = true,
     .check = CheckMissingForLoopStep},
    {.name = "ForeachLoopCondition",
     .enabled = true,
     .check = CheckForeachLoopCondition},
    {.name = "SelectInWeight", .enabled = true, .check = CheckSelectInWeight},
    {.name = "AssignmentPattern",
     .enabled = true,
     .check = CheckAssignmentPattern},
    {.name = "AssignmentPatternContext",
     .enabled = true,
     .check = CheckAssignmentPatternContext},
    {.name = "ScalarAssignmentPattern",
     .enabled = true,
     .check = CheckScalarAssignmentPattern},
    {.name = "TargetUnpackedArrayConcatenation",
     .enabled = true,
     .check = CheckTargetUnpackedArrayConcatenation},
    {.name = "InsideOperator", .enabled = true, .check = CheckInsideOperator},
    {.name = "InsideOperatorRange",
     .enabled = true,
     .check = CheckInsideOperatorRange},
    {.name = "TypeCasting", .enabled = true, .check = CheckTypeCasting},
    {.name = "TimeValue", .enabled = true, .check = CheckTimeValue},
    {.name = "MultipleBins", .enabled = true, .check = CheckMultipleBins},
    {.name = "AssertionstatementAttributeInstance",
     .enabled = true,
     .check = CheckAssertionStatementAttributeInstance},
    {.name = "SystemFunctionArguments",
     .enabled = true,
     .check = CheckSystemFunctionArguments},
    {.name = "WildcardOperator",
     .enabled = true,
     .check = CheckWildcardOperators},
    {.name = "ExponentFormatTimeValue",
     .enabled = true,
     .check = CheckExponentFormatTimeValue},
    {.name = "ExtendClass", .enabled = true, .check = CheckExtendClass},
    {.name = "DuplicateConstructor",
     .enabled = true,
     .check = CheckDuplicateConstructor},
    {.name = "DuplicateClass", .enabled = true, .check = CheckDuplicateClass},
    {.name = "ExternConstraintUndeclared",
     .enabled = true,
     .check = CheckExternConstraintUndeclared},
    {.name = "ExternFunctionUndeclared",
     .enabled = true,
     .check = CheckExternFunctionUndeclared},
    {.name = "ExternTaskUndeclared",
     .enabled = true,
     .check = CheckExternTaskUndeclared},
    {.name = "ExtendInterfaceClass",
     .enabled = true,
     .check = CheckExtendInterfaceClass},
    {.name = "ImplementClass", .enabled = true, .check = CheckImplementClass},
    {.name = "ImplementInterfaceClass",
     .enabled = true,
     .check = CheckImplementInterfaceClass},
    {.name = "CircularInheritance",
     .enabled = true,
     .check = CheckCircularInheritance},
});

static const std::array kGlobalRules = std::to_array<GlobalRule>({
    {.name = "NofParameterOverrides",
     .enabled = true,
     .check = CheckNofParameterOverrides},
    {.name = "MissingFunctionImplementation",
     .enabled = true,
     .check = CheckMissingFunctionImplementation},
    {.name = "FuctionImplementationScope",
     .enabled = true,
     .check = CheckFuncImplScope},
});

namespace {
void RunAllRules(const SL::FileContent* fileContent, SL::ErrorContainer* errors,
                 SL::SymbolTable* symbols) {
  for (const auto& rule : kAllRules) {
    if (!rule.enabled) {
      continue;
    }
    rule.check(fileContent, errors, symbols);
  }
}
}  // namespace

void RunAllRulesOnDesign(SL::Design* design, const vpiHandle& uhdmDesign,
                         SL::ErrorContainer* errors, SL::SymbolTable* symbols) {
  if (design == nullptr) {
    return;
  }

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
