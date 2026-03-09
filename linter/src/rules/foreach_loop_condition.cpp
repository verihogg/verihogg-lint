#include "rules/foreach_loop_condition.h"

#include <string_view>

#include "Surelog/Design/Design.h"
#include "Surelog/Design/FileContent.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"
#include "Surelog/SourceCompile/VObjectTypes.h"
#include "utils/location_utils.h"
#include "utils/name_utils.h"

using namespace SURELOG;

static int CountForeachDimensionGroups(const FileContent* fC,
                                       NodeId foreachKeyword) {
  NodeId arrayIdNode = FindArrayIdNode(fC, foreachKeyword);
  if (!arrayIdNode) return 0;

  int groups = 0;
  for (NodeId sib = fC->Sibling(arrayIdNode); sib; sib = fC->Sibling(sib)) {
    VObjectType t = fC->Type(sib);
    if (t == VObjectType::paLoop_variables || t == VObjectType::slStringConst)
      ++groups;
  }
  return groups;
}

void CheckForeachLoopCondition(const FileContent* fC, ErrorContainer* errors,
                               SymbolTable* symbols) {
  if (!fC || !errors || !symbols) return;

  NodeId root = fC->getRootNode();
  if (!root) return;

  for (NodeId foreachNode : fC->sl_collect_all(root, VObjectType::paFOREACH)) {
    if (!foreachNode) continue;

    if (CountForeachDimensionGroups(fC, foreachNode) <= 1) continue;

    NodeId arrayIdNode = FindArrayIdNode(fC, foreachNode);
    std::string_view arrayName =
        arrayIdNode ? ExtractName(fC, arrayIdNode, "unknown") : "unknown";

    ReportError(fC, foreachNode, arrayName,
                ErrorDefinition::LINT_FOREACH_LOOP_CONDITION, errors, symbols);
  }
}
