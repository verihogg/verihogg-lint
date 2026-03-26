#include "rules/select_in_event_control.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <algorithm>
#include <array>
#include <stack>

#include "main/lint_rules.h"
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

namespace {
auto EventExprHasEdge(const SL::FileContent* fileContent,
                      SL::NodeId eventExprId) -> bool {
  return std::ranges::any_of(kEdgeTypes, [&](SL::VObjectType type) {
    return !fileContent->sl_collect_all(eventExprId, type, false).empty();
  });
}

auto ContainsSelectInEventExpr(const SL::FileContent* fileContent,
                               SL::NodeId node) -> bool {
  if (!node) {
    return false;
  }

  std::stack<SL::NodeId> stack;
  stack.push(node);

  while (!stack.empty()) {
    SL::NodeId const kNode = stack.top();
    stack.pop();

    SL::VObjectType const kType = fileContent->Type(kNode);

    if (kType == SL::VObjectType::paEvent_expression &&
        EventExprHasEdge(fileContent, kNode)) {
      continue;
    }

    if (std::ranges::any_of(kSelectTypes, [kType](SL::VObjectType selectType) {
          return selectType == kType;
        })) {
      return true;
    }

    for (SL::NodeId child = fileContent->Child(kNode); child;
         child = fileContent->Sibling(child)) {
      stack.push(child);
    }
  }

  return false;
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
