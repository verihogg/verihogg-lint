#include "rules/select_in_weight.h"

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
    SL::NodeId const kNode = worklist.top();
    worklist.pop();

    SL::VObjectType const kType = fileContent->Type(kNode);

    if (kType == SL::VObjectType::paRs_code_block) {
      continue;
    }

    if (std::ranges::any_of(kSelectTypes, [kType](SL::VObjectType selectType) {
          return selectType == kType;
        })) {
      return true;
    }

    for (SL::NodeId child = fileContent->Child(kNode); child;
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

  SL::NodeId const kRoot = fileContent->getRootNode();
  if (!kRoot) {
    return;
  }

  for (SL::NodeId const kRsRuleId :
       fileContent->sl_collect_all(kRoot, SL::VObjectType::paRs_rule)) {
    SL::NodeId const kRsProdList = fileContent->Child(kRsRuleId);
    if (!kRsProdList) {
      continue;
    }
    if (fileContent->Type(kRsProdList) !=
        SL::VObjectType::paRs_production_list) {
      continue;
    }

    SL::NodeId const kWeightExpr = fileContent->Sibling(kRsProdList);
    if (!kWeightExpr) {
      continue;
    }
    if (fileContent->Type(kWeightExpr) != SL::VObjectType::paExpression) {
      continue;
    }

    if (ContainsSelectInExpr(fileContent, kWeightExpr)) {
      ReportError(fileContent, kRsRuleId,
                  ExtractName(fileContent, kRsProdList, "<unknown>"),
                  verihogg_lint::LINT_SELECT_IN_WEIGHT, errors, symbols);
    }
  }
}