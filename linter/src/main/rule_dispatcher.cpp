#include "main/rule_dispatcher.h"

#include <array>
#include <functional>

#include "Surelog/Common/FileSystem.h"
#include "Surelog/Design/FileContent.h"
#include "Surelog/SourceCompile/VObjectTypes.h"
#include "rules/all_rules.h"

using namespace SURELOG;

struct Rule {
  std::string name;
  bool enabled = true;
  std::function<void(const FileContent*, ErrorContainer*, SymbolTable*)> check;
};

static const std::array kAllRules = std::to_array<Rule>(
    {{"RepetitionInSequence", true, checkRepetitionInSequence},
     {"PrototypeReturnDataType", true, checkPrototypeReturnDataType},
     {"ParameterDynamicArray", true, checkParameterDynamicArray},
     {"ImplicitDataTypeInDeclaration", true,
      checkImplicitDataTypeInDeclaration},
     {"HierarchicalInterfaceIdentifier", true,
      checkHierarchicalInterfaceIdentifier},
     {"DpiDeclarationString", true, checkDpiDeclarationString},
     {"ClassVariableLifetime", true, checkClassVariableLifetime},
     {"CoverpointExpressionType", true, checkCoverpointExpressionType},
     {"CovergroupExpression", true, checkCovergroupExpression},
     {"ConcatenationMultiplier", true, checkConcatenationMultiplier},
     {"ParameterOverride", true, checkParameterOverride},
     {"MultipleDotStarConnections", true, checkMultipleDotStarConnections},
     {"SelectInEventControl", true, checkSelectInEventControl},
     {"EmptyAssignmentPattern", true, checkEmptyAssignmentPattern},
     {"MissingForLoopInitialization", true, checkMissingForLoopInitialization},
     {"MissingForLoopCondition", true, checkMissingForLoopCondition},
     {"MissingForLoopStep", true, checkMissingForLoopStep},
     {"ForeachLoopCondition", true, checkForeachLoopCondition},
     {"SelectInWeight", true, checkSelectInWeight},
     {"AssignmentPattern", true, checkAssignmentPattern},
     {"AssignmentPatternContext", true, checkAssignmentPatternContext},
     {"ScalarAssignmentPattern", true, checkScalarAssignmentPattern},
     {"TargetUnpackedArrayConcatenation", true,
      checkTargetUnpackedArrayConcatenation},
     {"InsideOperator", true, checkInsideOperator},
     {"InsideOperatorRange", true, checkInsideOperatorRange},
     {"TypeCasting", true, checkTypeCasting},
     {"TimeValue", true, checkTimeValue}

    });

void runAllRules(const FileContent* fC, ErrorContainer* errors,
                 SymbolTable* symbols) {
  for (const auto& rule : kAllRules) {
    if (!rule.enabled) continue;
    rule.check(fC, errors, symbols);
  }
}

void runAllRulesOnDesign(Design* design, const vpiHandle& UHDMdesign,
                         ErrorContainer* errors, SymbolTable* symbols) {
  if (!design) return;

  for (auto& [name, fC] : design->getAllFileContents()) {
    if (!fC) continue;

    runAllRules(fC, errors, symbols);
    FatalListener listener(fC, errors, symbols);
    listener.listen(UHDMdesign);
  }
}
