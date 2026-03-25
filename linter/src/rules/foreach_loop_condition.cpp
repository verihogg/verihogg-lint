#include "rules/foreach_loop_condition.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/Design.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <string_view>

#include "main/lint_rules.h"
#include "utils/location_utils.h"
#include "utils/name_utils.h"

namespace SL = SURELOG;

namespace {
auto CountForeachDimensionGroups(const SL::FileContent* fileContent,
                                 SL::NodeId foreachKeyword) -> int {
  SL::NodeId const arrayIdNode = FindArrayIdNode(fileContent, foreachKeyword);
  if (arrayIdNode == SL::InvalidNodeId) {
    return 0;
  }

  int groups = 0;
  for (SL::NodeId sib = fileContent->Sibling(arrayIdNode); sib;
       sib = fileContent->Sibling(sib)) {
    SL::VObjectType const type = fileContent->Type(sib);
    if (type == SL::VObjectType::paLoop_variables ||
        type == SL::VObjectType::slStringConst) {
      ++groups;
    }
  }
  return groups;
}
}  // namespace

void CheckForeachLoopCondition(const SL::FileContent* fileContent,
                               SL::ErrorContainer* errors,
                               SL::SymbolTable* symbols) {
  if (fileContent == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }

  SL::NodeId const root = fileContent->getRootNode();
  if (root == SL::InvalidNodeId) {
    return;
  }

  for (SL::NodeId const foreachNode :
       fileContent->sl_collect_all(root, SL::VObjectType::paFOREACH)) {
    if (foreachNode == SL::InvalidNodeId) {
      continue;
    }

    if (CountForeachDimensionGroups(fileContent, foreachNode) <= 1) {
      continue;
    }

    SL::NodeId const arrayIdNode = FindArrayIdNode(fileContent, foreachNode);
    std::string_view const arrayName =
        arrayIdNode ? ExtractName(fileContent, arrayIdNode, "unknown")
                    : "unknown";

    ReportError(fileContent, foreachNode, arrayName,
                verihogg_lint::LINT_FOREACH_LOOP_CONDITION, errors, symbols);
  }
}
