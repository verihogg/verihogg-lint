#include "rules/inside_operator_range.h"

#include <string>

#include "Surelog/Design/FileContent.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"
#include "Surelog/SourceCompile/VObjectTypes.h"
#include "utils/location_utils.h"

using namespace SURELOG;

static std::string getInsideContextName(const FileContent* fC,
                                        NodeId insideNode) {
  NodeId exprNode = fC->Parent(insideNode);
  if (!exprNode) return "<unknown>";

  NodeId leftOperand = fC->Child(exprNode);
  if (!leftOperand) return "<unknown>";

  NodeId current = leftOperand;
  while (current) {
    if (fC->Type(current) == VObjectType::slStringConst) {
      return std::string(fC->SymName(current));
    }
    current = fC->Child(current);
  }

  return "<unknown>";
}

static bool isValidInsideRange(const FileContent* fC, NodeId siblingNode) {
  if (!siblingNode) return false;

  VObjectType sibType = fC->Type(siblingNode);

  if (sibType == VObjectType::paOpen_range_list) {
    return true;
  }

  if (sibType == VObjectType::paExpression) {
    NodeId primaryNode = fC->Child(siblingNode);
    if (!primaryNode) return false;
    if (fC->Type(primaryNode) != VObjectType::paPrimary) return false;

    NodeId concatNode = fC->Child(primaryNode);
    if (!concatNode) return false;
    if (fC->Type(concatNode) == VObjectType::paConcatenation) return true;
  }

  return false;
}

void checkInsideOperatorRange(const FileContent* fC, ErrorContainer* errors,
                              SymbolTable* symbols) {
  if (!fC || !errors || !symbols) return;

  NodeId root = fC->getRootNode();

  auto insideNodes = fC->sl_collect_all(root, VObjectType::paINSIDE);

  for (NodeId insideId : insideNodes) {
    NodeId siblingId = fC->Sibling(insideId);

    if (!isValidInsideRange(fC, siblingId)) {
      std::string varName = getInsideContextName(fC, insideId);
      reportError(fC, insideId, varName,
                  ErrorDefinition::LINT_INSIDE_OPERATOR_RANGE, errors, symbols);
    }
  }
}
