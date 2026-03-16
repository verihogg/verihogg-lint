#include "rules/wildcard_operator.h"

#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include "utils/location_utils.h"
#include "utils/name_utils.h"

namespace SL = SURELOG;

void CheckWildcardOperators(const SL::FileContent* fileContent,
                            SL::ErrorContainer* errors,
                            SL::SymbolTable* symbols) {
  if (fileContent == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }

  SL::NodeId root = fileContent->getRootNode();
  if (!root) {
    return;
  }

  for (SL::NodeId wildEq :
       fileContent->sl_collect_all(root, SL::VObjectType::paBinOp_WildEqual)) {
    SL::NodeId exprNode = fileContent->Parent(wildEq);
    std::string_view symName = "<unknown>";

    if (exprNode) {
      SL::NodeId leftOperand = fileContent->Child(exprNode);
      if (leftOperand) {
        symName = ExtractName(fileContent, leftOperand, "<unknown>");
      }
    }

    ReportError(fileContent, wildEq, symName,
                SL::ErrorDefinition::LINT_WILDCARD_EQUALITY_OPERATOR, errors,
                symbols);

    ReportError(fileContent, wildEq, symName,
                SL::ErrorDefinition::LINT_WILDCARD_INEQUALITY_OPERATOR, errors,
                symbols);
  }
}