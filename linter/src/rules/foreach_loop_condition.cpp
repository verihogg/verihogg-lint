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
  SL::NodeId const kArrayIdNode = FindArrayIdNode(fileContent, foreachKeyword);
  if (kArrayIdNode == SL::InvalidNodeId) {
    return 0;
  }

  int groups = 0;
  for (SL::NodeId sib = fileContent->Sibling(kArrayIdNode); sib;
       sib = fileContent->Sibling(sib)) {
    SL::VObjectType const kType = fileContent->Type(sib);
    if (kType == SL::VObjectType::paLoop_variables ||
        kType == SL::VObjectType::slStringConst) {
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

  SL::NodeId const kRoot = fileContent->getRootNode();
  if (kRoot == SL::InvalidNodeId) {
    return;
  }

  for (SL::NodeId const kForeachNode :
       fileContent->sl_collect_all(kRoot, SL::VObjectType::paFOREACH)) {
    if (kForeachNode == SL::InvalidNodeId) {
      continue;
    }

    if (CountForeachDimensionGroups(fileContent, kForeachNode) <= 1) {
      continue;
    }

    SL::NodeId const kArrayIdNode = FindArrayIdNode(fileContent, kForeachNode);
    std::string_view const kArrayName =
        kArrayIdNode ? ExtractName(fileContent, kArrayIdNode, "unknown")
                     : "unknown";

    ReportError(fileContent, kForeachNode, kArrayName,
                verihogg_lint::LINT_FOREACH_LOOP_CONDITION, errors, symbols);
  }
}
