#include "rules/select_in_event_control.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <algorithm>
#include <array>

#include "main/lint_rules.h"
#include "utils/ast_utils.h"
#include "utils/location_utils.h"
#include "utils/name_utils.h"

namespace SL = SURELOG;

static constexpr std::array kEdgeTypes = {
    SL::VObjectType::paEdge_Posedge,
    SL::VObjectType::paEdge_Negedge,
};

namespace {
auto EventExprHasEdge(const SL::FileContent* fileContent,
                      SL::NodeId eventExprId) -> bool {
  return std::ranges::any_of(kEdgeTypes, [&](SL::VObjectType type) {
    return !fileContent->sl_collect_all(eventExprId, type, false).empty();
  });
}

auto ShouldPruneEdgeEventExpr(const SL::FileContent* fileContent,
                              SL::NodeId node, SL::VObjectType type) -> bool {
  return type == SL::VObjectType::paEvent_expression &&
         EventExprHasEdge(fileContent, node);
}

auto ContainsSelectInEventExpr(const SL::FileContent* fileContent,
                               SL::NodeId node) -> bool {
  return SubtreeContainsAnyType(
      fileContent, node,
      {SL::VObjectType::paSelect, SL::VObjectType::paConstant_select},
      ShouldPruneEdgeEventExpr);
}
}  // namespace

void CheckSelectInEventControl(const SL::FileContent* fileContent,
                               SL::ErrorContainer* errors,
                               SL::SymbolTable* symbols) {
  if (fileContent == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }

  SL::NodeId const kRoot = fileContent->getRootNode();
  if (!kRoot) {
    return;
  }

  for (SL::NodeId const kEventControlId :
       fileContent->sl_collect_all(kRoot, SL::VObjectType::paEvent_control)) {
    if (ContainsSelectInEventExpr(fileContent, kEventControlId)) {
      ReportError(fileContent, kEventControlId,
                  ExtractName(fileContent, kEventControlId),
                  verihogg_lint::LINT_SELECT_IN_EVENT_CONTROL, errors, symbols);
    }
  }
}
