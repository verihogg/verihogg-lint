#include "rule_dispatcher.h"

#include <iostream>

#include "Surelog/Common/FileSystem.h"
#include "Surelog/Design/FileContent.h"
#include "Surelog/SourceCompile/VObjectTypes.h"
#include "assignment_pattern.h"
#include "assignment_pattern_context.h"
#include "class_variable_lifetime.h"
#include "concatenation_multiplier.h"
#include "covergroup_expression.h"
#include "coverpoint_expression_type.h"
#include "dpi_decl_string.h"
#include "empty_assignment_pattern.h"
#include "fatal_rule.h"
#include "foreach_loop_condition.h"
#include "hierarchical_interface_identifier.h"
#include "implicit_data_type.h"
#include "inside_operator.h"
#include "inside_operator_range.h"
#include "missing_for_loop_condition.h"
#include "missing_for_loop_initialization.h"
#include "missing_for_loop_step.h"
#include "multiple_dot_star_connection.h"
#include "parameter_dynamic_array.h"
#include "parameter_override.h"
#include "prototype_return_data_type.h"
#include "repeat_in_sequence.h"
#include "scalar_assignment_pattern.h"
#include "select_in_event_control.h"
#include "select_in_weight.h"
#include "target_unpacked_array_concatenation.h"
#include "type_casting.h"

using namespace SURELOG;

void runAllRules(const FileContent* fC, ErrorContainer* errors,
                 SymbolTable* symbols) {
  Analyzer::checkRepetitionInSequence(fC, errors, symbols);
  Analyzer::checkPrototypeReturnDataType(fC, errors, symbols);
  Analyzer::checkParameterDynamicArray(fC, errors, symbols);
  Analyzer::checkImplicitDataTypeInDeclaration(fC, errors, symbols);
  Analyzer::checkHierarchicalInterfaceIdentifier(fC, errors, symbols);
  Analyzer::checkDpiDeclarationString(fC, errors, symbols);
  Analyzer::checkClassVariableLifetime(fC, errors, symbols);
  Analyzer::checkCoverpointExpressionType(fC, errors, symbols);
  Analyzer::checkCovergroupExpression(fC, errors, symbols);
  Analyzer::checkConcatenationMultiplier(fC, errors, symbols);
  Analyzer::checkParameterOverride(fC, errors, symbols);
  Analyzer::checkMultipleDotStarConnections(fC, errors, symbols);
  Analyzer::checkSelectInEventControl(fC, errors, symbols);
  Analyzer::checkEmptyAssignmentPattern(fC, errors, symbols);
  Analyzer::checkMissingForLoopInitialization(fC, errors, symbols);
  Analyzer::checkMissingForLoopCondition(fC, errors, symbols);
  Analyzer::checkMissingForLoopStep(fC, errors, symbols);
  Analyzer::checkForeachLoopCondition(fC, errors, symbols);
  Analyzer::checkSelectInWeight(fC, errors, symbols);
  Analyzer::checkAssignmentPattern(fC, errors, symbols);
  Analyzer::checkAssignmentPatternContext(fC, errors, symbols);
  Analyzer::checkScalarAssignmentPattern(fC, errors, symbols);
  Analyzer::checkTargetUnpackedArrayConcatenation(fC, errors, symbols);
  Analyzer::checkInsideOperator(fC, errors, symbols);
  Analyzer::checkInsideOperatorRange(fC, errors, symbols);
  Analyzer::checkTypeCasting(fC, errors, symbols);
}

void runAllRulesOnDesign(Design* design, const vpiHandle& UHDMdesign,
                         ErrorContainer* errors, SymbolTable* symbols) {
  if (!design) return;

  for (auto& it : design->getAllFileContents()) {
    const FileContent* fC = it.second;
    if (!fC) continue;

    runAllRules(fC, errors, symbols);
    FatalListener listener(fC, errors, symbols);
    listener.listen(UHDMdesign);
  }
}
