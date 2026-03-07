#include "rules/inside_operator_range.h"

#include <string_view>

#include "Surelog/Design/FileContent.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"
#include "Surelog/SourceCompile/VObjectTypes.h"
#include "utils/location_utils.h"

using namespace SURELOG;

static std::string_view getInsideContextName(const FileContent* fC,
                                             NodeId insideNode) {
  NodeId exprNode = fC->Parent(insideNode);
  if (!exprNode) return "<unknown>";

  NodeId leftOperand = fC->Child(exprNode);
  if (!leftOperand) return "<unknown>";

  auto stringNodes =
      fC->sl_collect_all(leftOperand, VObjectType::slStringConst);
  if (!stringNodes.empty())
    return std::string_view(fC->SymName(stringNodes.front()));

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
    if (!primaryNode || fC->Type(primaryNode) != VObjectType::paPrimary)
      return false;

    NodeId concatNode = fC->Child(primaryNode);
    if (concatNode && fC->Type(concatNode) == VObjectType::paConcatenation)
      return true;
  }

  return false;
}

void checkInsideOperatorRange(const FileContent* fC, ErrorContainer* errors,
                              SymbolTable* symbols) {
  if (!fC || !errors || !symbols) return;

  NodeId root = fC->getRootNode();
  if (!root) return;

  for (NodeId insideId : fC->sl_collect_all(root, VObjectType::paINSIDE)) {
    if (!isValidInsideRange(fC, fC->Sibling(insideId))) {
      reportError(fC, insideId, getInsideContextName(fC, insideId),
                  ErrorDefinition::LINT_INSIDE_OPERATOR_RANGE, errors, symbols);
    }
  }
}
