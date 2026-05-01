#include "rules/wildcard_inequality_operator.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <string_view>

#include "main/lint_rules.h"
#include "rules/wildcard_operator_common.h"
#include "utils/location_utils.h"

namespace SL = SURELOG;

void CheckWildcardInequalityOperator(const SL::FileContent* fileContent,
                                     SL::ErrorContainer* errors,
                                     SL::SymbolTable* symbols) {
  if (fileContent == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }

  const SL::NodeId root = fileContent->getRootNode();
  if (!root) {
    return;
  }

  for (const SL::NodeId node :
       fileContent->sl_collect_all(root, SL::VObjectType::paBinOp_WildEqual)) {
    if (DetectWildcardOperatorKind(fileContent, node) !=
        WildcardOperatorKind::kInequality) {
      continue;
    }

    const std::string_view symName = GetWildcardLhsName(fileContent, node);

    ReportError(fileContent, node, symName,
                verihogg_lint::LINT_WILDCARD_INEQUALITY_OPERATOR, errors,
                symbols);
  }
}
