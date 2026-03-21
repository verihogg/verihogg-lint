#pragma once
#include <algorithm>
#include <array>
#include <string_view>

#include "Surelog/ErrorReporting/ErrorDefinition.h"

namespace verihogg_lint {

using ED = SURELOG::ErrorDefinition;

[[nodiscard]] consteval auto LintId(int id) -> ED::ErrorType {
  return static_cast<ED::ErrorType>(id);
}

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

struct LintRuleInfo {
  ED::ErrorType type;
  ED::ErrorSeverity severity = ED::ERROR;
  ED::ErrorCategory category = ED::LINT;
  std::string_view text = {};
  std::string_view extraText = {};
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
};

inline void RegisterLintRules() {
  std::ranges::for_each(kLintRules, [](const LintRuleInfo& r) {
    ED::rec(r.type, r.severity, r.category, r.text, r.extraText);
  });
}

}  // namespace verihogg_lint