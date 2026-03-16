#include "main/rule_dispatcher.h"

#include <Surelog/Common/FileSystem.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <array>
#include <functional>
#include <string_view>

#include "rules/all_rules.h"

namespace SL = SURELOG;

struct Rule {
  std::string_view name;
  bool enabled = true;
  std::function<void(const SL::FileContent*, SL::ErrorContainer*,
                     SL::SymbolTable*)>
      check;
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
    {"ExponentFormatTimeValue", true, CheckExponentFormatTimeValue},

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
}
