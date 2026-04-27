#include "rules/type_casting.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <iostream>
#include <string_view>
#include <vector>

#include "fix/fix_it.h"
#include "main/lint_diagnostics.h"
#include "main/lint_rules.h"
#include "utils/ast_utils.h"
#include "utils/location_utils.h"
#include "utils/name_utils.h"

namespace SL = SURELOG;

auto CheckTypeCastingFixable(const SL::FileContent* fileContent,
                             SL::ErrorContainer* errors,
                             SL::SymbolTable* symbols,
                             [[maybe_unused]] FixSourceManager& sm)
    -> std::vector<LintDiagnostic> {
  std::vector<LintDiagnostic> diags;

  if (fileContent == nullptr || errors == nullptr || symbols == nullptr) {
    return diags;
  }

  SL::NodeId const kRoot = fileContent->getRootNode();
  if (!kRoot) {
    return diags;
  }

  auto userTypes = CollectUserDefinedTypes(fileContent, kRoot);
  if (userTypes.empty()) {
    return diags;
  }

  const std::string filepath = GetFixFilepath(fileContent);
  if (filepath.empty()) {
    return diags;
  }

  for (SL::NodeId const kFuncCallNode : fileContent->sl_collect_all(
           kRoot, SL::VObjectType::paComplex_func_call)) {
    if (fileContent->sl_get(kFuncCallNode, SL::VObjectType::paClass_scope)) {
      continue;
    }

    std::string_view const kTypeName = ExtractName(fileContent, kFuncCallNode);
    if (kTypeName.empty()) {
      continue;
    }

    if (!userTypes.contains(kTypeName)) {
      continue;
    }

    SL::NodeId const kNameNode = fileContent->Child(kFuncCallNode);
    ReportError(fileContent, kNameNode, kTypeName,
                verihogg_lint::LINT_TYPE_CASTING, errors, symbols);

    const unsigned line = fileContent->Line(kNameNode);
    const auto end_col =
        static_cast<unsigned>(fileContent->EndColumn(kNameNode));

    if (line == 0 || end_col == 0) {
      continue;
    }

    const FixLocation insert_pos{
        .filename = filepath, .line = line, .col = end_col};

    LintDiagnostic d;
    d.filepath = filepath;
    d.line = line;
    d.col = GetColumnSafe(fileContent, kNameNode);
    d.message = std::string(kTypeName) + "(...) -> " + std::string(kTypeName) +
                "'(...)";

    try {
      d.fixes.push_back(
          FixIt::Insert(insert_pos, "'", "add tick for type cast"));
    } catch (const std::exception& e) {
      std::cerr << "autofix: cannot create fix at " << filepath << ":" << line
                << ":" << end_col << ": " << e.what() << "\n";
      diags.push_back(std::move(d));
      continue;
    }

    diags.push_back(std::move(d));
  }

  return diags;
}
