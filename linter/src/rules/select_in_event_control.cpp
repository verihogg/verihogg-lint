#include "rules/select_in_event_control.h"

#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <algorithm>
#include <array>
#include <stack>
#include <string_view>

#include "utils/location_utils.h"
#include "utils/name_utils.h"

namespace SL = SURELOG;

static constexpr std::array kEdgeTypes = {
    SL::VObjectType::paEdge_Posedge,
    SL::VObjectType::paEdge_Negedge,
};

static constexpr std::array kSelectTypes = {
    SL::VObjectType::paSelect,
    SL::VObjectType::paConstant_select,
};

static auto EventExprHasEdge(const SL::FileContent* fileContent,
                             SL::NodeId eventExprId) -> bool {
  return std::ranges::any_of(kEdgeTypes, [&](SL::VObjectType type) {
    return !fileContent->sl_collect_all(eventExprId, type, false).empty();
  });
}

static auto ContainsSelectInEventExpr(const SL::FileContent* fileContent,
                                      SL::NodeId node) -> bool {
  if (!node) {
    return false;
  }

  std::stack<SL::NodeId> stack;
  stack.push(node);

  while (!stack.empty()) {
    SL::NodeId node = stack.top();
    stack.pop();

    SL::VObjectType type = fileContent->Type(node);

    if (type == SL::VObjectType::paEvent_expression &&
        EventExprHasEdge(fileContent, node)) {
      continue;
    }

    if (std::ranges::any_of(kSelectTypes, [type](SL::VObjectType selectType) {
          return selectType == type;
        })) {
      return true;
    }

    for (SL::NodeId child = fileContent->Child(node); child;
         child = fileContent->Sibling(child)) {
      stack.push(child);
    }
  }

  return false;
}

void CheckSelectInEventControl(const SL::FileContent* fileContent,
                               SL::ErrorContainer* errors,
                               SL::SymbolTable* symbols) {
  if (fileContent == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }

  SL::NodeId root = fileContent->getRootNode();
  if (!root) {
    return;
  }

  for (SL::NodeId eventControlId :
       fileContent->sl_collect_all(root, SL::VObjectType::paEvent_control)) {
    if (ContainsSelectInEventExpr(fileContent, eventControlId)) {
      ReportError(
          fileContent, eventControlId, ExtractName(fileContent, eventControlId),
          SL::ErrorDefinition::LINT_SELECT_IN_EVENT_CONTROL, errors, symbols);
    }
  }
}
