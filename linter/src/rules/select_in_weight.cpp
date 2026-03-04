#include "rules/select_in_weight.h"

#include <cstdint>
#include <string>

#include "Surelog/Design/Design.h"
#include "Surelog/Design/FileContent.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"
#include "Surelog/SourceCompile/VObjectTypes.h"
#include "utils/name_utils.h"
#include "utils/location_utils.h"

using namespace SURELOG;

static bool containsSelectInExpr(const FileContent* fC, NodeId node) {
  if (!fC || !node) return false;

  VObjectType t = fC->Type(node);

  if (t == VObjectType::paRs_code_block) return false;

  if (t == VObjectType::paSelect || t == VObjectType::paConstant_select)
    return true;

  for (NodeId child = fC->Child(node); child; child = fC->Sibling(child)) {
    if (containsSelectInExpr(fC, child)) return true;
  }

  return false;
}
void checkSelectInWeight(const FileContent* fC, ErrorContainer* errors,
                         SymbolTable* symbols) {
  if (!fC) return;

  NodeId root = fC->getRootNode();
  if (!root) return;
  auto rsRules = fC->sl_collect_all(root, VObjectType::paRs_rule);

  for (NodeId rsRuleId : rsRules) {
    NodeId rsProdList = fC->Child(rsRuleId);
    if (!rsProdList) continue;
    if (fC->Type(rsProdList) != VObjectType::paRs_production_list) continue;

    NodeId weightExpr = fC->Sibling(rsProdList);
    if (!weightExpr) continue;
    if (fC->Type(weightExpr) != VObjectType::paExpression) continue;

    if (containsSelectInExpr(fC, weightExpr)) {
      std::string name = extractName(fC, rsProdList, "<unknown>");
      reportError(fC, rsRuleId, name, ErrorDefinition::LINT_SELECT_IN_WEIGHT,
                  errors, symbols);
    }
  }
}