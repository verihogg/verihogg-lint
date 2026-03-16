#include "rules/foreach_loop_condition.h"

#include <Surelog/Design/Design.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <string_view>

#include "utils/location_utils.h"
#include "utils/name_utils.h"

namespace SL = SURELOG;

static auto CountForeachDimensionGroups(const SL::FileContent* fileContent,
                                        SL::NodeId foreachKeyword) -> int {
  SL::NodeId arrayIdNode = FindArrayIdNode(fileContent, foreachKeyword);
  if (arrayIdNode == SL::InvalidNodeId) {
    return 0;
  }

  int groups = 0;
  for (SL::NodeId sib = fileContent->Sibling(arrayIdNode); sib;
       sib = fileContent->Sibling(sib)) {
    SL::VObjectType type = fileContent->Type(sib);
    if (type == SL::VObjectType::paLoop_variables ||
        type == SL::VObjectType::slStringConst) {
      ++groups;
    }
  }
  return groups;
}

void CheckForeachLoopCondition(const SL::FileContent* fileContent,
                               SL::ErrorContainer* errors,
                               SL::SymbolTable* symbols) {
  if (fileContent == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }

  SL::NodeId root = fileContent->getRootNode();
  if (root == SL::InvalidNodeId) {
    return;
  }

  for (SL::NodeId foreachNode :
       fileContent->sl_collect_all(root, SL::VObjectType::paFOREACH)) {
    if (foreachNode == SL::InvalidNodeId) {
      continue;
    }

    if (CountForeachDimensionGroups(fileContent, foreachNode) <= 1) {
      continue;
    }

    SL::NodeId arrayIdNode = FindArrayIdNode(fileContent, foreachNode);
    std::string_view arrayName =
        arrayIdNode ? ExtractName(fileContent, arrayIdNode, "unknown")
                    : "unknown";

    ReportError(fileContent, foreachNode, arrayName,
                SL::ErrorDefinition::LINT_FOREACH_LOOP_CONDITION, errors,
                symbols);
  }
}
