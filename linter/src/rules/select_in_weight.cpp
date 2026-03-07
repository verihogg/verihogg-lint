#include "rules/select_in_weight.h"

#include <algorithm>
#include <array>

#include "Surelog/Design/FileContent.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"
#include "Surelog/SourceCompile/VObjectTypes.h"
#include "utils/location_utils.h"
#include "utils/name_utils.h"

using namespace SURELOG;

static constexpr std::array kSelectTypes = {
    VObjectType::paSelect,
    VObjectType::paConstant_select,
};

static bool containsSelectInExpr(const FileContent* fC, NodeId node) {
  if (!node) return false;

  VObjectType t = fC->Type(node);

  if (t == VObjectType::paRs_code_block) return false;

  if (std::ranges::any_of(kSelectTypes,
                          [t](VObjectType st) { return st == t; }))
    return true;

  for (NodeId child = fC->Child(node); child; child = fC->Sibling(child)) {
    if (containsSelectInExpr(fC, child)) return true;
  }

  return false;
}
void checkSelectInWeight(const FileContent* fC, ErrorContainer* errors,
                         SymbolTable* symbols) {
  if (!fC || !errors || !symbols) return;

  NodeId root = fC->getRootNode();
  if (!root) return;

  for (NodeId rsRuleId : fC->sl_collect_all(root, VObjectType::paRs_rule)) {
    NodeId rsProdList = fC->Child(rsRuleId);
    if (!rsProdList) continue;
    if (fC->Type(rsProdList) != VObjectType::paRs_production_list) continue;

    NodeId weightExpr = fC->Sibling(rsProdList);
    if (!weightExpr) continue;
    if (fC->Type(weightExpr) != VObjectType::paExpression) continue;

    if (containsSelectInExpr(fC, weightExpr)) {
      reportError(fC, rsRuleId, extractName(fC, rsProdList, "<unknown>"),
                  ErrorDefinition::LINT_SELECT_IN_WEIGHT, errors, symbols);
    }
  }
}