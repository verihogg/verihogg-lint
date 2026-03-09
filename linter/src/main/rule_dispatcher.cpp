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

static const std::array kAllRules = std::to_array<Rule>({
    {"RepetitionInSequence", true, CheckRepetitionInSequence},
    {"PrototypeReturnDataType", true, CheckPrototypeReturnDataType},
    {"ParameterDynamicArray", true, CheckParameterDynamicArray},
    {"ImplicitDataTypeInDeclaration", true, CheckImplicitDataTypeInDeclaration},
    {"HierarchicalInterfaceIdentifier", true,
     CheckHierarchicalInterfaceIdentifier},
    {"DpiDeclarationString", true, CheckDpiDeclarationString},
    {"ClassVariableLifetime", true, CheckClassVariableLifetime},
    {"CoverpointExpressionType", true, CheckCoverpointExpressionType},
    {"CovergroupExpression", true, CheckCovergroupExpression},
    {"ConcatenationMultiplier", true, CheckConcatenationMultiplier},
    {"ParameterOverride", true, CheckParameterOverride},
    {"MultipleDotStarConnections", true, CheckMultipleDotStarConnections},
    {"SelectInEventControl", true, CheckSelectInEventControl},
    {"EmptyAssignmentPattern", true, CheckEmptyAssignmentPattern},
    {"MissingForLoopInitialization", true, CheckMissingForLoopInitialization},
    {"MissingForLoopCondition", true, CheckMissingForLoopCondition},
    {"MissingForLoopStep", true, CheckMissingForLoopStep},
    {"ForeachLoopCondition", true, CheckForeachLoopCondition},
    {"SelectInWeight", true, CheckSelectInWeight},
    {"AssignmentPattern", true, CheckAssignmentPattern},
    {"AssignmentPatternContext", true, CheckAssignmentPatternContext},
    {"ScalarAssignmentPattern", true, CheckScalarAssignmentPattern},
    {"TargetUnpackedArrayConcatenation", true,
     CheckTargetUnpackedArrayConcatenation},
    {"InsideOperator", true, CheckInsideOperator},
    {"InsideOperatorRange", true, CheckInsideOperatorRange},
    {"TypeCasting", true, CheckTypeCasting},
    {"TimeValue", true, CheckTimeValue},
    {"MultipleBins", true, CheckMultipleBins},
    {"AssertionstatementAttributeInstance", true,
     CheckAssertionStatementAttributeInstance},
    {"SystemFunctionArguments", true, CheckSystemFunctionArguments},
    {"WildcardOperator", true, CheckWildcardOperators},

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
    listener.Listen(UHDMdesign);
  }
}
