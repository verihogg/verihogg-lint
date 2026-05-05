#include "rules/wildcard_equality_operator.h"

#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>

#include <vector>

#include "fix/source_manager.h"
#include "main/lint_diagnostics.h"
#include "main/lint_rules.h"
#include "rules/wildcard_operator_common.h"

namespace SL = SURELOG;

auto CheckWildcardEqualityOperatorFixable(const SL::FileContent* fileContent,
                                          SL::ErrorContainer* errors,
                                          SL::SymbolTable* symbols,
                                          FixSourceManager& /*sm*/)
    -> std::vector<LintDiagnostic> {
  return CheckWildcardOperatorFixableImpl(
      fileContent, errors, symbols, WildcardOperatorKind::kEquality,
      verihogg_lint::LINT_WILDCARD_EQUALITY_OPERATOR,
      {.message = "=?= -> ==?", .correctOp = "==?"});
}
