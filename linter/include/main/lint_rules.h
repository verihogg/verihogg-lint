#pragma once
#include <Surelog/ErrorReporting/ErrorDefinition.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <string_view>

namespace verihogg_lint {

using ED = SURELOG::ErrorDefinition;

// NOLINTBEGIN(cppcoreguidelines-use-enum-class)
enum LintIdEnum : uint16_t {
  LINT_CLASS_VARIABLE_LIFETIME = 734,
  LINT_DPI_DECLARATION_STRING,
  LINT_HIERARCHICAL_INTERFACE_IDENTIFIER,
  LINT_IMPLICIT_DATA_TYPE,
  LINT_PARAMETR_DYNAMIC_ARRAY,
  LINT_PROTOTYPE_RETURN_DATA_TYPE,
  LINT_REPETITION_IN_SEQUENCE,
  LINT_FATAL_SYSTEM_TASK_FIRST_ARGUMENT,
  LINT_COVERPOINT_EXPRESSION_TYPE,
  LINT_COVERGROUP_EXPRESSION,
  LINT_CONCATENATION_MULTIPLIER,
  LINT_PARAMETR_OVERRIDE,
  LINT_MULTIPLE_DOT_STAR_CONNECTIONS,
  LINT_SELECT_IN_EVENT_CONTROL,
  LINT_EMPTY_ASSIGNMENT_PATTERN,
  LINT_MISSING_FOR_LOOP_INITIALIZATION,
  LINT_MISSING_FOR_LOOP_CONDITION,
  LINT_MISSING_FOR_LOOP_STEP,
  LINT_FOREACH_LOOP_CONDITION,
  LINT_SELECT_IN_WEIGHT,
  LINT_ASSIGNMENT_PATTERN,
  LINT_ASSIGNMENT_PATTERN_CONTEXT,
  LINT_SCALAR_ASSIGNMENT_PATTERN,
  LINT_TARGET_UNPACKED_ARRAY_CONCATENATION,
  LINT_INSIDE_OPERATOR,
  LINT_INSIDE_OPERATOR_RANGE,
  LINT_TYPE_CASTING,
  LINT_TIME_VALUE,
  LINT_MULTIPLE_BINS,
  LINT_ASSERTION_STATEMENT_ATTRIBUTE_INSTANCE,
  LINT_SYSTEM_FUNCTION_ARGUMENTS,
  LINT_WILDCARD_EQUALITY_OPERATOR,
  LINT_WILDCARD_INEQUALITY_OPERATOR,
  LINT_EXPONENT_FORMAT_TIME_VALUE,
  LINT_NOF_PARAMETER_OVERRIDE,
  LINT_MISSING_FUNCTION_IMPLEMENTATION,
  LINT_MISSING_TASK_IMPLEMENTATION,
  LINT_FUNC_IMPL_SCOPE,
  LINT_TASK_IMPL_SCOPE,
  LINT_CONSTRAINT_IMPL_SCOPE,
  LINT_EXTEND_CLASS,
  LINT_DUPLICATE_CONSTRUCTOR,
  LINT_DUPLICATE_CLASS,
  LINT_EXTERN_CONSTRAINT_UNDECLARED,
  LINT_EXTERN_FUNCTION_UNDECLARED,
  LINT_EXTERN_TASK_UNDECLARED,
  LINT_EXTEND_INTERFACE_CLASS,
  LINT_IMPLEMENT_CLASS,
  LINT_IMPLEMENT_INTERFACE_CLASS,
  LINT_CIRCULAR_INHERITANCE,
  LINT_MODPORT_IMPORT_EXPORT_PORT,
  LINT_EVENT_CONTROL_EXPRESSION,
  LINT_METHOD_OVERRIDE_ARGUMENT_NAME,
  LINT_FUNCTION_IMPLEMENTATION_RETURN_TYPE,
  LINT_FUNCTION_IMPLEMENTATION_INTERNAL_RETURN_TYPE,
  LINT_METHOD_IMPLEMENTATION_ARGUMENT_TYPE,
  LINT_VOID_CAST_OF_VOID_FUNCTION,
  LINT_LOGICAL_NEGATION,
  LINT_INCOMPLETE_ASSIGNMENT_PATTERN,
  LINT_ASSIGNMENT_PATTERN_VALUES,
  LINT_INVALID_LIBLIST,
  LINT_UNDECLARED_CELL,
  LINT_UNDECLARED_DESIGN,
  LINT_UNDECLARED_CONFIGURATION,
  LINT_DUPLICATE_EVENT,
  LINT_EVENT_SINGULAR,
  LINT_DUPLICATE_ENUM_LITERAL,
};
// NOLINTEND(cppcoreguidelines-use-enum-class)

class LintId {
 public:
  // NOLINTBEGIN(google-explicit-constructor, hicpp-explicit-conversions)
  /*implicit*/ constexpr LintId(LintIdEnum e) : e(e) {}

  /*implicit*/ constexpr operator LintIdEnum() const { return e; }
  /*implicit*/ constexpr operator ED::ErrorType() const {
    return static_cast<ED::ErrorType>(e);
  }
  // NOLINTEND(google-explicit-constructor, hicpp-explicit-conversions)
  explicit constexpr operator int() const { return static_cast<int>(e); }
  explicit operator bool() const = delete;

 private:
  LintIdEnum e;
};

// NOLINTBEGIN(readability-identifier-naming)

// NOLINTEND(readability-identifier-naming)
struct LintRuleInfo {
  verihogg_lint::LintId type;
  ED::ErrorSeverity severity = ED::ERROR;
  ED::ErrorCategory category = ED::LINT;
  std::string_view text;
};

inline constexpr std::array kLintRules = {
    LintRuleInfo{.type = LINT_CLASS_VARIABLE_LIFETIME,
                 .text = "Class variable '%s' cannot use automatic lifetime"},
    LintRuleInfo{.type = LINT_DPI_DECLARATION_STRING,
                 .text = R"(expecting "DPI-C" instead of "%s")"},
    LintRuleInfo{
        .type = LINT_HIERARCHICAL_INTERFACE_IDENTIFIER,
        .text = "hierarchical interface identifier '%s' is not allowed"},
    LintRuleInfo{.type = LINT_IMPLICIT_DATA_TYPE,
                 .text = "variable \"%s\" declared without explicit type"},
    LintRuleInfo{
        .type = LINT_PARAMETR_DYNAMIC_ARRAY,
        .text =
            "parameter \"%s\" uses unsized (dynamic) unpacked array dimension"},
    LintRuleInfo{.type = LINT_PROTOTYPE_RETURN_DATA_TYPE,
                 .text = "Function prototype \"%s\" missing return data type"},
    LintRuleInfo{.type = LINT_REPETITION_IN_SEQUENCE,
                 .text = "sequence \"%s\" uses both goto '[->]' and "
                         "non-consecutive '[=]' repetitions"},
    LintRuleInfo{.type = LINT_FATAL_SYSTEM_TASK_FIRST_ARGUMENT,
                 .text = "$fatal system call violation: %s"},
    LintRuleInfo{
        .type = LINT_COVERPOINT_EXPRESSION_TYPE,
        .text = "Coverpoint expression should be of an integral data type: %s"},
    LintRuleInfo{.type = LINT_COVERGROUP_EXPRESSION,
                 .text = "Covergroup expression should be a literal or "
                         "covergroup argument: %s"},
    LintRuleInfo{
        .type = LINT_CONCATENATION_MULTIPLIER,
        .text = "Concatenation multiplier should be a constant expression: %s"},
    LintRuleInfo{.type = LINT_PARAMETR_OVERRIDE,
                 .text = "Expecting parentheses around parameter override: %s"},
    LintRuleInfo{.type = LINT_MULTIPLE_DOT_STAR_CONNECTIONS,
                 .text = "Dot star port connection '.*' cannot appear more "
                         "than once in port list: %s"},
    LintRuleInfo{.type = LINT_SELECT_IN_EVENT_CONTROL,
                 .text = "Select in event control not allowed: %s"},
    LintRuleInfo{.type = LINT_EMPTY_ASSIGNMENT_PATTERN,
                 .text = "Empty assignment pattern '{}' not allowed: %s"},
    LintRuleInfo{.type = LINT_MISSING_FOR_LOOP_INITIALIZATION,
                 .text = "'for' loop variable initialization required: %s"},
    LintRuleInfo{.type = LINT_MISSING_FOR_LOOP_CONDITION,
                 .text = "'for' loop conditional expression required: %s"},
    LintRuleInfo{.type = LINT_MISSING_FOR_LOOP_STEP,
                 .text = "'for' loop step expression required: %s"},
    LintRuleInfo{.type = LINT_FOREACH_LOOP_CONDITION,
                 .text = "Multidimensional array select not allowed in foreach "
                         "loop condition: %s"},
    LintRuleInfo{.type = LINT_SELECT_IN_WEIGHT,
                 .text = "Select in weight specification not allowed: %s"},
    LintRuleInfo{.type = LINT_ASSIGNMENT_PATTERN,
                 .text = "Expecting assignment pattern '{...}' instead of "
                         "concatenation: %s"},
    LintRuleInfo{.type = LINT_ASSIGNMENT_PATTERN_CONTEXT,
                 .text =
                     "Assignment pattern not allowed outside assignment-like "
                     "context (could not determine data type): %s"},
    LintRuleInfo{.type = LINT_SCALAR_ASSIGNMENT_PATTERN,
                 .text = "Variable of 1-bit scalar type not allowed as target "
                         "of assignment pattern: %s"},
    LintRuleInfo{.type = LINT_TARGET_UNPACKED_ARRAY_CONCATENATION,
                 .text = "Unpacked array concatenation not allowed as target "
                         "expression: %s"},
    LintRuleInfo{
        .type = LINT_INSIDE_OPERATOR,
        .text = "'inside' operator in constant expression not allowed: %s"},
    LintRuleInfo{
        .type = LINT_INSIDE_OPERATOR_RANGE,
        .text = "Expecting curly braces {} around 'inside' operator range: %s"},
    LintRuleInfo{.type = LINT_TYPE_CASTING,
                 .text = "Expecting tick before type casting expression: %s"},
    LintRuleInfo{
        .type = LINT_TIME_VALUE,
        .text = "Unexpected white space between number and time value: %s"},
    LintRuleInfo{
        .type = LINT_MULTIPLE_BINS,
        .text = "Specification of multiple bins dimension not allowed: %s"},
    LintRuleInfo{.type = LINT_ASSERTION_STATEMENT_ATTRIBUTE_INSTANCE,
                 .text = "Expecting attribute instance after block identifier "
                         "# for procedural assertion statement: %s"},
    LintRuleInfo{.type = LINT_SYSTEM_FUNCTION_ARGUMENTS,
                 .text = "Maximum number of arguments for %s"},
    LintRuleInfo{
        .type = LINT_WILDCARD_EQUALITY_OPERATOR,
        .text = "Expecting wildcard operator '==?' instead of '=?=': %s"},
    LintRuleInfo{
        .type = LINT_WILDCARD_INEQUALITY_OPERATOR,
        .text = "Expecting wildcard operator '!=?' instead of '!?=': %s"},
    LintRuleInfo{.type = LINT_EXPONENT_FORMAT_TIME_VALUE,
                 .text = "Unexpected exponent format for time value: %s"},
    LintRuleInfo{
        .type = LINT_NOF_PARAMETER_OVERRIDE,
        .text = "Expected # parameter overrides, found #module %s; endmodule"},
    LintRuleInfo{.type = LINT_MISSING_FUNCTION_IMPLEMENTATION,
                 .text = "extern function is not implemented: %s"},
    LintRuleInfo{.type = LINT_MISSING_TASK_IMPLEMENTATION,
                 .text = "extern task is not implemented: %s"},
    LintRuleInfo{
        .type = LINT_FUNC_IMPL_SCOPE,
        .text = "extern function implemented outside of its class scope: %s"},
    LintRuleInfo{
        .type = LINT_TASK_IMPL_SCOPE,
        .text = "extern task implemented outside of its class scope: %s"},
    LintRuleInfo{
        .type = LINT_CONSTRAINT_IMPL_SCOPE,
        .text = "extern constraint implemented outside of its class scope: %s"},

    LintRuleInfo{.type = LINT_EXTEND_CLASS,
                 .text = "Extending non existing class %s"},
    LintRuleInfo{.type = LINT_DUPLICATE_CONSTRUCTOR,
                 .text = "Duplicate constructor %s already declared"},
    LintRuleInfo{.type = LINT_DUPLICATE_CLASS,
                 .text = "Duplicate class %s, already declared"},
    LintRuleInfo{
        .type = LINT_EXTERN_CONSTRAINT_UNDECLARED,
        .text =
            "Outer class constraint was not declared extern inside class %s"},
    LintRuleInfo{
        .type = LINT_EXTERN_FUNCTION_UNDECLARED,
        .text = "Outer class function was not declared extern inside class %s"},
    LintRuleInfo{
        .type = LINT_EXTERN_TASK_UNDECLARED,
        .text = "Outer class task was not declared extern inside class %s"},
    LintRuleInfo{
        .type = LINT_EXTEND_INTERFACE_CLASS,
        .text =
            "Extending interface class %s by non-interface class not allowed"},
    LintRuleInfo{
        .type = LINT_IMPLEMENT_CLASS,
        .text = "Implementing non-interface class %s by class not allowed"},
    LintRuleInfo{.type = LINT_IMPLEMENT_INTERFACE_CLASS,
                 .text = "Implementing non existing interface class %s"},
    LintRuleInfo{.type = LINT_CIRCULAR_INHERITANCE,
                 .text = "Class %s extends itself"},
    LintRuleInfo{
        .type = LINT_MODPORT_IMPORT_EXPORT_PORT,
        .text = "еxpected method name instead of interface signal name: %s"},
    LintRuleInfo{.type = LINT_EVENT_CONTROL_EXPRESSION,
                 .text = "еxpected singular data type for event control "
                         "expression instead of: %s"},
    LintRuleInfo{
        .type = LINT_METHOD_OVERRIDE_ARGUMENT_NAME,
        .text = "argument name of method does not match of override: %s"},
    LintRuleInfo{.type = LINT_FUNCTION_IMPLEMENTATION_RETURN_TYPE,
                 .text = "return type of function must be the same as "
                         "prototype return type: %s"},
    LintRuleInfo{
        .type = LINT_FUNCTION_IMPLEMENTATION_INTERNAL_RETURN_TYPE,
        .text = "Internal return type for the implementation of extern method "
                "requires scope resolution: %s"},
    LintRuleInfo{
        .type = LINT_METHOD_IMPLEMENTATION_ARGUMENT_TYPE,
        .text =
            "Argument type of method must be the same as prototype argument "
            "type (non-standard use of type alias): %s"},
    LintRuleInfo{.type = LINT_VOID_CAST_OF_VOID_FUNCTION,
                 .text = "void cast of void function not allowed: %s"},
    LintRuleInfo{.type = LINT_LOGICAL_NEGATION,
                 .text =
                     "Operand of type '%s' not allowed with logical negation "
                     "(use == null instead)"},
    LintRuleInfo{.type = LINT_INCOMPLETE_ASSIGNMENT_PATTERN,
                 .text = "Incomplete named assignment pattern: %s"},
    LintRuleInfo{.type = LINT_ASSIGNMENT_PATTERN_VALUES, .text = "%s"},
    LintRuleInfo{.type = LINT_INVALID_LIBLIST,
                 .text = "Liblist is empty for %s"},
    LintRuleInfo{.type = LINT_UNDECLARED_CELL,
                 .text = "Cell %s is not declared"},
    LintRuleInfo{.type = LINT_UNDECLARED_DESIGN,
                 .text = "Top design %s is not declared"},
    LintRuleInfo{.type = LINT_UNDECLARED_CONFIGURATION,
                 .text = "Configuration %s is not declared"},
    LintRuleInfo{.type = LINT_DUPLICATE_EVENT, .text = "Duplicate event: %s"},
    LintRuleInfo{.type = LINT_EVENT_SINGULAR,
                 .text = "Events must be singular: %s"},
    LintRuleInfo{.type = LINT_DUPLICATE_ENUM_LITERAL,
                 .text = "Duplicate enum literal: %s"},
};

inline void RegisterLintRules() {
  std::ranges::for_each(kLintRules, [](const LintRuleInfo& rule) {
    ED::rec(rule.type, rule.severity, rule.category, rule.text);
  });
}

}  // namespace verihogg_lint
