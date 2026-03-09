#include "rules/wildcard_operator.h"

#include "Surelog/Design/FileContent.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"
#include "Surelog/SourceCompile/VObjectTypes.h"
#include "utils/location_utils.h"
#include "utils/name_utils.h"

using namespace SURELOG;

void checkWildcardOperators(const FileContent* fC, ErrorContainer* errors,
                            SymbolTable* symbols) {
  if (!fC || !errors || !symbols) return;

  NodeId root = fC->getRootNode();
  if (!root) return;

  for (NodeId wildEq :
       fC->sl_collect_all(root, VObjectType::paBinOp_WildEqual)) {
    NodeId exprNode = fC->Parent(wildEq);
    std::string_view symName = "<unknown>";

    if (exprNode) {
      NodeId leftOperand = fC->Child(exprNode);
      if (leftOperand) {
        symName = extractName(fC, leftOperand, "<unknown>");
      }
    }

    reportError(fC, wildEq, symName,
                ErrorDefinition::LINT_WILDCARD_EQUALITY_OPERATOR, errors,
                symbols);

    reportError(fC, wildEq, symName,
                ErrorDefinition::LINT_WILDCARD_INEQUALITY_OPERATOR, errors,
                symbols);
  }
}