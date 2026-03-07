#include "rules/select_in_event_control.h"

#include <algorithm>
#include <array>
#include <string_view>

#include "Surelog/Design/FileContent.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"
#include "Surelog/SourceCompile/VObjectTypes.h"
#include "utils/location_utils.h"
#include "utils/name_utils.h"

using namespace SURELOG;

static constexpr std::array kEdgeTypes = {
    VObjectType::paEdge_Posedge,
    VObjectType::paEdge_Negedge,
};

static constexpr std::array kSelectTypes = {
    VObjectType::paSelect,
    VObjectType::paConstant_select,
};

static bool eventExprHasEdge(const FileContent* fC, NodeId eventExprId) {
  return std::ranges::any_of(kEdgeTypes, [&](VObjectType t) {
    return !fC->sl_collect_all(eventExprId, t, false).empty();
  });
}

static bool containsSelectInEventExpr(const FileContent* fC, NodeId node) {
  if (!node) return false;

  VObjectType t = fC->Type(node);

  if (t == VObjectType::paEvent_expression && eventExprHasEdge(fC, node))
    return false;

  if (std::ranges::any_of(kSelectTypes,
                          [t](VObjectType st) { return st == t; }))
    return true;

  for (NodeId child = fC->Child(node); child; child = fC->Sibling(child)) {
    if (containsSelectInEventExpr(fC, child)) return true;
  }

  return false;
}

void checkSelectInEventControl(const FileContent* fC, ErrorContainer* errors,
                               SymbolTable* symbols) {
  if (!fC || !errors || !symbols) return;

  NodeId root = fC->getRootNode();
  if (!root) return;

  for (NodeId eventControlId :
       fC->sl_collect_all(root, VObjectType::paEvent_control)) {
    if (containsSelectInEventExpr(fC, eventControlId)) {
      reportError(fC, eventControlId, extractName(fC, eventControlId),
                  ErrorDefinition::LINT_SELECT_IN_EVENT_CONTROL, errors,
                  symbols);
    }
  }
}
