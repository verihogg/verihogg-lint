#pragma once
#include <Surelog/ErrorReporting/ErrorDefinition.h>

#include <algorithm>
#include <array>
#include <string_view>

namespace verihogg_lint {

using ED = SURELOG::ErrorDefinition;

[[nodiscard]] consteval auto LintId(int ident) -> ED::ErrorType {
  return static_cast<ED::ErrorType>(ident);
}

// NOLINTBEGIN(readability-identifier-naming)
inline constexpr ED::ErrorType LINT_CLASS_VARIABLE_LIFETIME = LintId(734);
inline constexpr ED::ErrorType LINT_DPI_DECLARATION_STRING = LintId(735);
inline constexpr ED::ErrorType LINT_HIERARCHICAL_INTERFACE_IDENTIFIER =
    LintId(736);
inline constexpr ED::ErrorType LINT_IMPLICIT_DATA_TYPE = LintId(737);
inline constexpr ED::ErrorType LINT_PARAMETR_DYNAMIC_ARRAY = LintId(738);
inline constexpr ED::ErrorType LINT_PROTOTYPE_RETURN_DATA_TYPE = LintId(739);
inline constexpr ED::ErrorType LINT_REPETITION_IN_SEQUENCE = LintId(740);
inline constexpr ED::ErrorType LINT_FATAL_SYSCALL = LintId(741);
inline constexpr ED::ErrorType LINT_COVERPOINT_EXPRESSION_TYPE = LintId(742);
inline constexpr ED::ErrorType LINT_COVERGROUP_EXPRESSION = LintId(743);
inline constexpr ED::ErrorType LINT_CONCATENATION_MULTIPLIER = LintId(744);
inline constexpr ED::ErrorType LINT_PARAMETR_OVERRIDE = LintId(745);
inline constexpr ED::ErrorType LINT_MULTIPLE_DOT_STAR_CONNECTIONS = LintId(746);
inline constexpr ED::ErrorType LINT_SELECT_IN_EVENT_CONTROL = LintId(747);
inline constexpr ED::ErrorType LINT_EMPTY_ASSIGNMENT_PATTERN = LintId(748);
inline constexpr ED::ErrorType LINT_MISSING_FOR_LOOP_INITIALIZATION =
    LintId(749);
inline constexpr ED::ErrorType LINT_MISSING_FOR_LOOP_CONDITION = LintId(750);
inline constexpr ED::ErrorType LINT_MISSING_FOR_LOOP_STEP = LintId(751);
inline constexpr ED::ErrorType LINT_FOREACH_LOOP_CONDITION = LintId(752);
inline constexpr ED::ErrorType LINT_SELECT_IN_WEIGHT = LintId(753);
inline constexpr ED::ErrorType LINT_ASSIGNMENT_PATTERN = LintId(754);
inline constexpr ED::ErrorType LINT_ASSIGNMENT_PATTERN_CONTEXT = LintId(755);
inline constexpr ED::ErrorType LINT_SCALAR_ASSIGNMENT_PATTERN = LintId(756);
inline constexpr ED::ErrorType LINT_TARGET_UNPACKED_ARRAY_CONCATENATION =
    LintId(757);
inline constexpr ED::ErrorType LINT_INSIDE_OPERATOR = LintId(758);
inline constexpr ED::ErrorType LINT_INSIDE_OPERATOR_RANGE = LintId(759);
inline constexpr ED::ErrorType LINT_TYPE_CASTING = LintId(760);
inline constexpr ED::ErrorType LINT_TIME_VALUE = LintId(761);
inline constexpr ED::ErrorType LINT_MULTIPLE_BINS = LintId(762);
inline constexpr ED::ErrorType LINT_ASSERTION_STATEMENT_ATTRIBUTE_INSTANCE =
    LintId(763);
inline constexpr ED::ErrorType LINT_SYSTEM_FUNCTION_ARGUMENTS = LintId(764);
inline constexpr ED::ErrorType LINT_WILDCARD_EQUALITY_OPERATOR = LintId(765);
inline constexpr ED::ErrorType LINT_WILDCARD_INEQUALITY_OPERATOR = LintId(766);
inline constexpr ED::ErrorType LINT_EXPONENT_FORMAT_TIME_VALUE = LintId(767);
inline constexpr ED::ErrorType LINT_NOF_PARAMETER_OVERRIDE = LintId(768);
inline constexpr ED::ErrorType LINT_MISSING_FUNCTION_IMPLEMENTATION =
    LintId(769);
inline constexpr ED::ErrorType LINT_MISSING_TASK_IMPLEMENTATION = LintId(770);
inline constexpr ED::ErrorType LINT_FUNC_IMPL_SCOPE = LintId(771);
inline constexpr ED::ErrorType LINT_TASK_IMPL_SCOPE = LintId(772);
inline constexpr ED::ErrorType LINT_CONSTRAINT_IMPL_SCOPE = LintId(773);

inline constexpr ED::ErrorType LINT_EXTEND_CLASS = LintId(774);
inline constexpr ED::ErrorType LINT_DUPLICATE_CONSTRUCTOR = LintId(775);
inline constexpr ED::ErrorType LINT_DUPLICATE_CLASS = LintId(776);
inline constexpr ED::ErrorType LINT_EXTERN_CONSTRAINT_UNDECLARED = LintId(777);
inline constexpr ED::ErrorType LINT_EXTERN_FUNCTION_UNDECLARED = LintId(778);
inline constexpr ED::ErrorType LINT_EXTERN_TASK_UNDECLARED = LintId(779);
inline constexpr ED::ErrorType LINT_EXTEND_INTERFACE_CLASS = LintId(780);
inline constexpr ED::ErrorType LINT_IMPLEMENT_CLASS = LintId(781);
inline constexpr ED::ErrorType LINT_IMPLEMENT_INTERFACE_CLASS = LintId(782);
inline constexpr ED::ErrorType LINT_CIRCULAR_INHERITANCE = LintId(783);
// NOLINTEND(readability-identifier-naming)
struct LintRuleInfo {
  ED::ErrorType type;
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
    LintRuleInfo{.type = LINT_FATAL_SYSCALL,
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
};

inline void RegisterLintRules() {
  std::ranges::for_each(kLintRules, [](const LintRuleInfo& rule) {
    ED::rec(rule.type, rule.severity, rule.category, rule.text);
  });
}

}  // namespace verihogg_lint
