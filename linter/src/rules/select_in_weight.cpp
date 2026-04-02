#include "rules/select_in_weight.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include "main/lint_rules.h"
#include "utils/ast_utils.h"
#include "utils/location_utils.h"
#include "utils/name_utils.h"

namespace SL = SURELOG;

namespace {
auto ShouldPruneRsCodeBlock(const SL::FileContent* fileContent, SL::NodeId node,
                            SL::VObjectType type) -> bool {
  (void)fileContent;
  (void)node;
  return type == SL::VObjectType::paRs_code_block;
}

auto ContainsSelectInExpr(const SL::FileContent* fileContent, SL::NodeId node)
    -> bool {
  return SubtreeContainsAnyType(
      fileContent, node,
      {SL::VObjectType::paSelect, SL::VObjectType::paConstant_select},
      ShouldPruneRsCodeBlock);
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