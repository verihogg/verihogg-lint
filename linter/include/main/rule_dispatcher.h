#pragma once

#include <Surelog/Design/Design.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <uhdm/vpi_user.h>

#include <array>
#include <filesystem>
#include <functional>
#include <string_view>

#include "main/lint_rules.h"
#include "rules/assertion_statement_atribute_instance.h"
#include "rules/assignment_pattern.h"
#include "rules/assignment_pattern_context.h"
#include "rules/assignment_pattern_values.h"
#include "rules/circular_inheritance.h"
#include "rules/class_variable_lifetime.h"
#include "rules/concatenation_multiplier.h"
#include "rules/constraint_implemention_scope.h"
#include "rules/covergroup_expression.h"
#include "rules/coverpoint_expression_type.h"
#include "rules/dpi_decl_string.h"
#include "rules/duplicate_class.h"
#include "rules/duplicate_constructor.h"
#include "rules/duplicate_cover_cross.h"
#include "rules/duplicate_enum_literal.h"
#include "rules/duplicate_event.h"
#include "rules/empty_assignment_pattern.h"
#include "rules/event_control_expression.h"
#include "rules/event_singular.h"
#include "rules/exponent_format_time_value.h"
#include "rules/extend_class.h"
#include "rules/extend_interface_class.h"
#include "rules/extern_constraint_undeclared.h"
#include "rules/extern_function_undeclared.h"
#include "rules/extern_task_undeclared.h"
#include "rules/fatal_rule.h"
#include "rules/foreach_loop_condition.h"
#include "rules/function_implementation_internal_return_type.h"
#include "rules/function_implementation_return_type.h"
#include "rules/function_implemention_scope.h"
#include "rules/hierarchical_interface_identifier.h"
#include "rules/implement_class.h"
#include "rules/implement_interface_class.h"
#include "rules/implicit_data_type.h"
#include "rules/incomplete_assignment_pattern.h"
#include "rules/inside_operator.h"
#include "rules/inside_operator_range.h"
#include "rules/invalid_liblist.h"
#include "rules/logical_negation.h"
#include "rules/method_implementation_argument_type.h"
#include "rules/method_override_argument_name.h"
#include "rules/missing_for_loop_condition.h"
#include "rules/missing_for_loop_initialization.h"
#include "rules/missing_for_loop_step.h"
#include "rules/missing_function_implementation.h"
#include "rules/missing_task_implementation.h"
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
#include "rules/task_implemention_scope.h"
#include "rules/time_value.h"
#include "rules/type_casting.h"
#include "rules/undeclared_cell.h"
#include "rules/undeclared_configuration.h"
#include "rules/undeclared_design.h"
#include "rules/void_cast_of_void_function.h"
#include "rules/wildcard_equality_operator.h"
#include "rules/wildcard_inequality_operator.h"
constexpr const char* DefaultConfigFileName = ".verihogg-lint";

namespace RuleInfo {
struct Rule {
  std::string_view idName;
  std::string_view description;
  std::function<void(const SURELOG::FileContent*, SURELOG::ErrorContainer*,
                     SURELOG::SymbolTable*)>
      check;
};

struct GlobalRule {
  std::string_view idName;
  std::string_view description;
  std::function<void(SURELOG::Design*, SURELOG::ErrorContainer*,
                     SURELOG::SymbolTable*)>
      check;
};

struct UhdmRule {
  std::string_view idName;
  std::string_view description;
  std::function<void(const vpiHandle&, SURELOG::ErrorContainer*,
                     SURELOG::SymbolTable*)>
      check;
};

const auto allRules = std::to_array<Rule>({
    {.idName = "ASSERTION_STATEMENT_ATTRIBUTE_INSTANCE",
     .description = "Expecting attribute instance after block identifier "
                    "for assertion",
     .check = CheckAssertionStatementAttributeInstance},
    {.idName = "ASSIGNMENT_PATTERN",
     .description =
         "Expecting assignment pattern '{...}' instead of concatenation",
     .check = CheckAssignmentPattern},
    {.idName = "ASSIGNMENT_PATTERN_CONTEXT",
     .description =
         "Assignment pattern not allowed outside assignment-like context",
     .check = CheckAssignmentPatternContext},
    {.idName = "CIRCULAR_INHERITANCE",
     .description = "Classes cannot have circular inheritance",
     .check = CheckCircularInheritance},
    {.idName = "CLASS_VARIABLE_LIFETIME",
     .description = "'automatic' lifetime for class variable not allowed",
     .check = CheckClassVariableLifetime},
    {.idName = "CONCATENATION_MULTIPLIER",
     .description = "Expecting constant expression as concatenation multiplier",
     .check = CheckConcatenationMultiplier},
    {.idName = "COVERPOINT_EXPRESSION",
     .description =
         "Expecting constant expression or non-ref covergroup argument",
     .check = CheckCovergroupExpression},
    {.idName = "MULTIPLE_BINS",
     .description = "Specification of multiple bins dimension not allowed",
     .check = CheckMultipleBins},
    {.idName = "COVERPOINT_EXPRESSION_TYPE",
     .description = "Coverpoint expression should be of an integral data type",
     .check = CheckCoverpointExpressionType},
    {.idName = "DPI_DECLARATION_STRING",
     .description = R"(Expecting "DPI" or "DPI-C")",
     .check = CheckDpiDeclarationString},
    {.idName = "DUPLICATE_CLASS",
     .description = "duplicate class, already declared",
     .check = CheckDuplicateClass},
    {.idName = "DUPLICATE_CONSTRUCTOR",
     .description = "duplicate constructor already declared",
     .check = CheckDuplicateConstructor},
    {.idName = "EMPTY_ASSIGNMENT_PATTERN",
     .description = "Empty assignment pattern '{}' not allowed",
     .check = CheckEmptyAssignmentPattern},
    {.idName = "EXPONENT_FORMAT_TIME_VALUE",
     .description = "Unexpected exponent format for time value",
     .check = CheckExponentFormatTimeValue},
    {.idName = "EXTEND_CLASS",
     .description = "extending non existing class",
     .check = CheckExtendClass},
    {.idName = "MODPORT_IMPORT_EXPORT_PORT",
     .description = "еxpected method name instead of interface signal name",
     .check = CheckModportImportExportPort},
    {.idName = "EVENT_CONTROL_EXPRESSION",
     .description = "еxpected singular data type for event control expression "
                    "instead of type",
     .check = CheckEventControlExpression},
    {.idName = "EXTEND_INTERFACE_CLASS",
     .description =
         "extending interface class by non-interface class not allowed",
     .check = CheckExtendInterfaceClass},
    {.idName = "EXTERN_CONSTRAINT_UNDECLARED",
     .description = "outer class constraint was not declared extern",
     .check = CheckExternConstraintUndeclared},
    {.idName = "EXTERN_FUNCTION_UNDECLARED",
     .description = "outer class function was not declared extern",
     .check = CheckExternFunctionUndeclared},
    {.idName = "EXTERN_TASK_UNDECLARED",
     .description = "outer class task was not declared extern",
     .check = CheckExternTaskUndeclared},
    {.idName = "FOREACH_LOOP_CONDITION",
     .description = "Multidimensional array select not allowed in foreach "
                    "loop condition",
     .check = CheckForeachLoopCondition},
    {.idName = "HIERARCHICAL_INTERFACE_IDENTIFIER",
     .description = "Hierarchical interface identifier not allowed",
     .check = CheckHierarchicalInterfaceIdentifier},
    {.idName = "IMPLEMENT_CLASS",
     .description = "implementing non-interface class by class not allowed",
     .check = CheckImplementClass},
    {.idName = "IMPLEMENT_INTERFACE_CLASS",
     .description = "implementing non existing interface class",
     .check = CheckImplementInterfaceClass},
    {.idName = "IMPLICIT_DATA_TYPE_IN_DECLARATION",
     .description =
         "Expecting net type (e.g. wire) or 'var' before implicit data type",
     .check = CheckImplicitDataTypeInDeclaration},
    {.idName = "INSIDE_OPERATOR",
     .description = "'inside' operator in constant expression not allowed",
     .check = CheckInsideOperator},
    {.idName = "INSIDE_OPERATOR_RANGE",
     .description = "Expecting curly braces {} around 'inside' operator range",
     .check = CheckInsideOperatorRange},
    {.idName = "MISSING_FOR_LOOP_CONDITION",
     .description = "'for' loop conditional expression required",
     .check = CheckMissingForLoopCondition},
    {.idName = "MISSING_FOR_LOOP_INITIALIZATION",
     .description = "'for' loop variable initialization required",
     .check = CheckMissingForLoopInitialization},
    {.idName = "MISSING_FOR_LOOP_STEP",
     .description = "'for' loop step required",
     .check = CheckMissingForLoopStep},
    {.idName = "MULTIPLE_DOT_STAR_CONNECTIONS",
     .description = "'.*' cannot appear more than once in the port list",
     .check = CheckMultipleDotStarConnections},
    {.idName = "PARAMETER_DYNAMIC_ARRAY",
     .description = "Fixed size required for parameter dimension",
     .check = CheckParameterDynamicArray},
    {.idName = "PARAMETER_OVERRIDE",
     .description = "Expecting parentheses around parameter override",
     .check = CheckParameterOverride},
    {.idName = "PROTOTYPE_RETURN_DATA_TYPE",
     .description = "Expecting return data type or void for function prototype",
     .check = CheckPrototypeReturnDataType},
    {.idName = "REPETITION_IN_SEQUENCE",
     .description = "Goto '[-> and non-consecutive '[= operators not allowed",
     .check = CheckRepetitionInSequence},
    {.idName = "SCALAR_ASSIGNMENT_PATTERN",
     .description = "Variable of 1-bit scalar type not allowed as assignment "
                    "pattern target",
     .check = CheckScalarAssignmentPattern},
    {.idName = "LINT_INCOMPLETE_ASSIGNMENT_PATTERN",
     .description =
         "Checks named struct assignment patterns for missing members",
     .check = CheckIncompleteAssignmentPattern},
    {.idName = "ASSIGNMENT_PATTERN_VALUES",
     .description =
         "Checks positional assignment patterns for element count "
         "mismatch against the target type (struct or packed vector)",
     .check = CheckAssignmentPatternValues},
    {.idName = "SELECT_IN_EVENT_CONTROL",
     .description = "Select in event control not allowed",
     .check = CheckSelectInEventControl},
    {.idName = "SELECT_IN_WEIGHT",
     .description = "Select in weight specification not allowed",
     .check = CheckSelectInWeight},
    {.idName = "SYSTEM_FUNCTION_ARGUMENTS",
     .description = "Maximum number of arguments for system function exceeded",
     .check = CheckSystemFunctionArguments},
    {.idName = "TARGET_UNPACKED_ARRAY_CONCATENATION",
     .description =
         "Unpacked array concatenation not allowed as target expression",
     .check = CheckTargetUnpackedArrayConcatenation},
    {.idName = "TIME_VALUE",
     .description = "Unexpected white space between number and time value",
     .check = CheckTimeValue},
    {.idName = "TYPE_CASTING",
     .description = "Expecting tick before type casting expression",
     .check = CheckTypeCasting},
    {.idName = "WILDCARD_EQUALITY_OPERATOR",
     .description = "Expecting wildcard operator '==?' instead of '=?='",
     .check = CheckWildcardEqualityOperator},
    {.idName = "WILDCARD_INEQUALITY_OPERATOR",
     .description = "Expecting wildcard operator '!=?' instead of '!?='",
     .check = CheckWildcardInequalityOperator},
    {.idName = "VOID_CAST_OF_VOID_FUNCTION",
     .description = "void cast of void function not allowed",
     .check = CheckVoidCastOfVoidFunction},
    {.idName = "LOGICAL_NEGATION",
     .description = "Operand of type not allowed with logical negation (use == "
                    "null instead)",
     .check = CheckLogicalNegation},
    {.idName = "INVALID_LIBLIST",
     .description = "Liblist must have at least one entry",
     .check = CheckInvalidLiblist},
});

constexpr size_t AllRulesSize = allRules.size();

const auto globalRules = std::to_array<GlobalRule>({
    {.idName = "NOF_PARAMETER_OVERRIDES",
     .description = "Expected # parameter overrides, found #module; endmodule",
     .check = CheckNofParameterOverrides},
    {.idName = "MISSING_FUNCTION_IMPLEMENTATION",
     .description = "extern function is not implemented",
     .check = CheckMissingFunctionImplementation},
    {.idName = "MISSING_TASK_IMPLEMENTATION",
     .description = "extern task is not implemented",
     .check = CheckMissingTaskImplementation},
    {.idName = "FUNCTION_IMPLEMENTATION_SCOPE",
     .description = "extern function implemented outside of its class scope",
     .check = CheckFuncImplScope},
    {.idName = "TASK_IMPLEMENTATION_SCOPE",
     .description = "extern task implemented outside of its class scope",
     .check = CheckTaskImplScope},
    {.idName = "CONSTRAINT_IMPLEMENTATION_SCOPE",
     .description = "extern constraint implemented outside of its class scope",
     .check = CheckConstraintImplScope},
    {.idName = "METHOD_OVERRIDE_ARGUMENT_NAME",
     .description = "argument name of method does not match of override",
     .check = CheckMethodOverrideArgumentName},
    {.idName = "FUNCTION_IMPLEMENTATION_RETURN_TYPE",
     .description =
         "return type of function must be the same as prototype return type",
     .check = CheckFunctionImplementationReturnType},
    {.idName = "FUNCTION_IMPLEMENTATION_INTERNAL_RETURN_TYPE",
     .description = "Internal return type for the implementation of extern "
                    "method requires scope resolution",
     .check = CheckFunctionImplementationInternalReturnType},
    {.idName = "METHOD_IMPLEMENTATION_ARGUMENT_TYPE",
     .description = "Argument type of method must be the same as prototype "
                    "argument type (non-standard use of type alias)",
     .check = CheckMethodImplementationArgumentType},
    {.idName = "UNDECLARED_CELL",
     .description = "Cell must be declared",
     .check = CheckUndeclaredCell},
    {.idName = "UNDECLARED_DESIGN",
     .description = "Design must be declared",
     .check = CheckUndeclaredDesign},
    {.idName = "UNDECLARED_CONFIGURATION",
     .description = "Configuration must be declared",
     .check = CheckUndeclaredConfiguration},
    {.idName = "DUPLICATE_EVENT",
     .description = "Duplicate event #, already declared at line # file #",
     .check = CheckDuplicateEvents},
    {.idName = "ILLEGAL_EVENT",
     .description = "The following events must be of a singular data type:#",
     .check = CheckEventSingular},
    {.idName = "DUPLICATE_ENUM_LITERAL",
     .description =
         "Duplicate enumeration literal #, already declared at line # file #",
     .check = CheckDuplicateEnumLiteral},
    {.idName = "DUPLICATE_COVER_CROSS",
     .description =
         "Duplicate cover cross #, already declared at line # file #",
     .check = CheckDuplicateCoverCross},
});

constexpr size_t AllGlobalRulesSize = globalRules.size();

const auto uhdmRules = std::to_array<UhdmRule>({
    {.idName = "FATAL_SYSTEM_TASK_FIRST_ARGUMENT",
     .description = "$fatal system call violation",
     .check = CheckFatalSyscall},
});

constexpr size_t AllUhdmRulesSize = uhdmRules.size();

constexpr size_t TotalRuleCount =
    AllRulesSize + AllGlobalRulesSize + AllUhdmRulesSize;

static_assert(TotalRuleCount == verihogg_lint::kLintRules.size());
}  // namespace RuleInfo

void RunAllRulesOnDesign(SURELOG::Design* design, const vpiHandle& uhdmDesign,
                         SURELOG::ErrorContainer* errors,
                         SURELOG::SymbolTable* symbols,
                         const std::filesystem::path& configFile);
