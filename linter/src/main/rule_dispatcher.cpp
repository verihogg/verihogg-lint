#include "main/rule_dispatcher.h"

#include <iostream>

#include "Surelog/Common/FileSystem.h"
#include "Surelog/Design/FileContent.h"
#include "Surelog/SourceCompile/VObjectTypes.h"
#include "rules/all_rules.h"

using namespace SURELOG;

void runAllRules(const FileContent* fC, ErrorContainer* errors,
                 SymbolTable* symbols) {
  checkRepetitionInSequence(fC, errors, symbols);
  checkPrototypeReturnDataType(fC, errors, symbols);
  checkParameterDynamicArray(fC, errors, symbols);
  checkImplicitDataTypeInDeclaration(fC, errors, symbols);
  checkHierarchicalInterfaceIdentifier(fC, errors, symbols);
  checkDpiDeclarationString(fC, errors, symbols);
  checkClassVariableLifetime(fC, errors, symbols);
  checkCoverpointExpressionType(fC, errors, symbols);
  checkCovergroupExpression(fC, errors, symbols);
  checkConcatenationMultiplier(fC, errors, symbols);
  checkParameterOverride(fC, errors, symbols);
  checkMultipleDotStarConnections(fC, errors, symbols);
  checkSelectInEventControl(fC, errors, symbols);
  checkEmptyAssignmentPattern(fC, errors, symbols);
  checkMissingForLoopInitialization(fC, errors, symbols);
  checkMissingForLoopCondition(fC, errors, symbols);
  checkMissingForLoopStep(fC, errors, symbols);
  checkForeachLoopCondition(fC, errors, symbols);
  checkSelectInWeight(fC, errors, symbols);
  checkAssignmentPattern(fC, errors, symbols);
  checkAssignmentPatternContext(fC, errors, symbols);
  checkScalarAssignmentPattern(fC, errors, symbols);
  checkTargetUnpackedArrayConcatenation(fC, errors, symbols);
  checkInsideOperator(fC, errors, symbols);
  checkInsideOperatorRange(fC, errors, symbols);
  checkTypeCasting(fC, errors, symbols);
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
