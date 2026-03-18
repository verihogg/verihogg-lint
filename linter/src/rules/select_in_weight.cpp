#include "rules/select_in_weight.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/ErrorReporting/ErrorDefinition.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <algorithm>
#include <array>
#include <stack>

#include "utils/location_utils.h"
#include "utils/name_utils.h"

namespace SL = SURELOG;

static constexpr std::array kSelectTypes = {
    SL::VObjectType::paSelect,
    SL::VObjectType::paConstant_select,
};

namespace {
auto ContainsSelectInExpr(const SL::FileContent* fileContent, SL::NodeId node)
    -> bool {
  if (!node) {
    return false;
  }

  std::stack<SL::NodeId> worklist;
  worklist.push(node);

  while (!worklist.empty()) {
    SL::NodeId const node = worklist.top();
    worklist.pop();

    SL::VObjectType const type = fileContent->Type(node);

    if (type == SL::VObjectType::paRs_code_block) {
      continue;
    }

    if (std::ranges::any_of(kSelectTypes, [type](SL::VObjectType selectType) {
          return selectType == type;
        })) {
      return true;
    }

    for (SL::NodeId child = fileContent->Child(node); child;
         child = fileContent->Sibling(child)) {
      worklist.push(child);
    }
  }

  return false;
}
}  // namespace
void CheckSelectInWeight(const SL::FileContent* fileContent,
                         SL::ErrorContainer* errors, SL::SymbolTable* symbols) {
  if (fileContent == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }

  SL::NodeId const root = fileContent->getRootNode();
  if (!root) {
    return;
  }

  for (SL::NodeId const rsRuleId :
       fileContent->sl_collect_all(root, SL::VObjectType::paRs_rule)) {
    SL::NodeId const rsProdList = fileContent->Child(rsRuleId);
    if (!rsProdList) {
      continue;
    }
    if (fileContent->Type(rsProdList) !=
        SL::VObjectType::paRs_production_list) {
      continue;
    }

    SL::NodeId const weightExpr = fileContent->Sibling(rsProdList);
    if (!weightExpr) {
      continue;
    }
    if (fileContent->Type(weightExpr) != SL::VObjectType::paExpression) {
      continue;
    }

    if (ContainsSelectInExpr(fileContent, weightExpr)) {
      ReportError(fileContent, rsRuleId,
                  ExtractName(fileContent, rsProdList, "<unknown>"),
                  SL::ErrorDefinition::LINT_SELECT_IN_WEIGHT, errors, symbols);
    }
  }
}