#include "rules/wildcard_operator.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <string_view>

#include "main/lint_rules.h"
#include "utils/location_utils.h"
#include "utils/name_utils.h"

namespace SL = SURELOG;

void CheckWildcardOperators(const SL::FileContent* fileContent,
                            SL::ErrorContainer* errors,
                            SL::SymbolTable* symbols) {
  if (fileContent == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }

  SL::NodeId const kRoot = fileContent->getRootNode();
  if (!kRoot) {
    return;
  }

  for (SL::NodeId const kWildEq :
       fileContent->sl_collect_all(kRoot, SL::VObjectType::paBinOp_WildEqual)) {
    SL::NodeId const kExprNode = fileContent->Parent(kWildEq);
    std::string_view symName = "<unknown>";

    if (kExprNode) {
      SL::NodeId const kLeftOperand = fileContent->Child(kExprNode);
      if (kLeftOperand) {
        symName = ExtractName(fileContent, kLeftOperand, "<unknown>");
      }
    }

    ReportError(fileContent, kWildEq, symName,
                verihogg_lint::LINT_WILDCARD_EQUALITY_OPERATOR, errors,
                symbols);

    ReportError(fileContent, kWildEq, symName,
                verihogg_lint::LINT_WILDCARD_INEQUALITY_OPERATOR, errors,
                symbols);
  }
}