#include "rules/wildcard_inequality_operator.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include "fix/fix_it.h"
#include "fix/source_manager.h"
#include "main/lint_diagnostics.h"
#include "main/lint_rules.h"
#include "rules/wildcard_operator_common.h"
#include "utils/location_utils.h"

namespace SL = SURELOG;

auto CheckWildcardInequalityOperatorFixable(const SL::FileContent* fileContent,
                                            SL::ErrorContainer* errors,
                                            SL::SymbolTable* symbols,
                                            FixSourceManager& /*sm*/)
    -> std::vector<LintDiagnostic> {
  std::vector<LintDiagnostic> diags;

  if (fileContent == nullptr || errors == nullptr || symbols == nullptr) {
    return diags;
  }

  const SL::NodeId kRoot = fileContent->getRootNode();
  if (!kRoot) {
    return diags;
  }

  const std::string filepath = GetFixFilepath(fileContent);
  if (filepath.empty()) {
    return diags;
  }

  for (const SL::NodeId kWildEq :
       fileContent->sl_collect_all(kRoot, SL::VObjectType::paBinOp_WildEqual)) {
    if (DetectWildcardOperatorKind(fileContent, kWildEq) !=
        WildcardOperatorKind::kInequality) {
      continue;
    }

    const std::string_view symName = GetWildcardLhsName(fileContent, kWildEq);

    ReportError(fileContent, kWildEq, symName,
                verihogg_lint::LINT_WILDCARD_INEQUALITY_OPERATOR, errors,
                symbols);

    const unsigned line = fileContent->Line(kWildEq);
    const unsigned col = GetColumnSafe(fileContent, kWildEq);
    if (line == 0 || col == 0) {
      continue;
    }

    const FixRange op_range{
        .begin = FixLocation{.filename = filepath, .line = line, .col = col},
        .end = FixLocation{.filename = filepath, .line = line, .col = col + 3},
    };

    LintDiagnostic d;
    d.filepath = filepath;
    d.line = line;
    d.col = col;
    d.message = "!?= -> !=?";

    try {
      d.fixes.push_back(
          FixIt::Replace(op_range, "!=?", "replace '!?=' with '!=?'"));
    } catch (const std::exception& e) {
      std::cerr << "autofix: cannot create fix at " << filepath << ":" << line
                << ":" << col << ": " << e.what() << "\n";
      diags.push_back(std::move(d));
      continue;
    }

    diags.push_back(std::move(d));
  }

  return diags;
}
