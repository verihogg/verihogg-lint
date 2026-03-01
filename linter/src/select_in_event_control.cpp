#include "select_in_event_control.h"

#include <cstdint>
#include <string>

#include "Surelog/Design/Design.h"
#include "Surelog/Design/FileContent.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"
#include "Surelog/SourceCompile/VObjectTypes.h"
#include "linter_utils.h"

using namespace SURELOG;

namespace Analyzer {

static bool eventExprHasEdge(const FileContent* fC, NodeId eventExprId) {
  for (NodeId child = fC->Child(eventExprId); child;
       child = fC->Sibling(child)) {
    VObjectType t = fC->Type(child);
    if (t == VObjectType::paEdge_Posedge || t == VObjectType::paEdge_Negedge)
      return true;
  }
  return false;
}

static bool containsSelectInEventExpr(const FileContent* fC, NodeId node) {
  if (!fC || !node) return false;

  VObjectType t = fC->Type(node);

  if (t == VObjectType::paEvent_expression && eventExprHasEdge(fC, node))
    return false;

  if (t == VObjectType::paSelect || t == VObjectType::paConstant_select)
    return true;

  for (NodeId child = fC->Child(node); child; child = fC->Sibling(child)) {
    if (containsSelectInEventExpr(fC, child)) return true;
  }

  return false;
}

void checkSelectInEventControl(const FileContent* fC, ErrorContainer* errors,
                               SymbolTable* symbols) {
  if (!fC) return;

  NodeId root = fC->getRootNode();
  if (!root) return;

  auto eventControls = fC->sl_collect_all(root, VObjectType::paEvent_control);

  for (NodeId eventControlId : eventControls) {
    if (containsSelectInEventExpr(fC, eventControlId)) {
      std::string name = extractName(fC, eventControlId);
      reportError(fC, eventControlId, name,
                  ErrorDefinition::LINT_SELECT_IN_EVENT_CONTROL, errors,
                  symbols);
    }
  }
}

}  // namespace Analyzer
