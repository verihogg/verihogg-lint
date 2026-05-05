#include "rules/wildcard_operator_common.h"

#include <Surelog/Common/FileSystem.h>
#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/ErrorReporting/ErrorDefinition.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include "fix/fix_it.h"
#include "fix/source_manager.h"
#include "main/lint_diagnostics.h"
#include "utils/location_utils.h"
#include "utils/name_utils.h"

namespace SL = SURELOG;

namespace {

auto ReadWildcardOperatorLexeme(const SL::FileContent* fileContent,
                                SL::NodeId opNode) -> std::string {
  if (fileContent == nullptr || !opNode) {
    return {};
  }

  const unsigned line = fileContent->Line(opNode);
  const unsigned col = fileContent->Column(opNode);
  if (line == 0 || col == 0) {
    return {};
  }

  SURELOG::FileSystem* const fileSystem = SURELOG::FileSystem::getInstance();

  std::vector<std::string> lines;
  if (!fileSystem->readLines(fileContent->getFileId(), lines)) {
    return {};
  }

  if (line > lines.size()) {
    return {};
  }

  const std::string& srcLine = lines.at(line - 1);
  const auto begin = static_cast<size_t>(col - 1);
  if (begin + 3 > srcLine.size()) {
    return {};
  }

  return srcLine.substr(begin, 3);
}

}  // namespace

auto GetWildcardLhsName(const SL::FileContent* fileContent, SL::NodeId opNode)
    -> std::string_view {
  std::string_view symName = "<unknown>";

  if (fileContent == nullptr || !opNode) {
    return symName;
  }

  const SL::NodeId exprNode = fileContent->Parent(opNode);
  if (!exprNode) {
    return symName;
  }

  const SL::NodeId leftOperand = fileContent->Child(exprNode);
  if (!leftOperand) {
    return symName;
  }

  return ExtractName(fileContent, leftOperand, "<unknown>");
}

auto DetectWildcardOperatorKind(const SL::FileContent* fileContent,
                                SL::NodeId opNode) -> WildcardOperatorKind {
  const std::string op = ReadWildcardOperatorLexeme(fileContent, opNode);
  if (op.size() != 3) {
    return WildcardOperatorKind::kUnknown;
  }

  if (op.at(1) != '?' || op.at(2) != '=') {
    return WildcardOperatorKind::kUnknown;
  }

  if (op.at(0) == '=') {
    return WildcardOperatorKind::kEquality;
  }

  if (op.at(0) == '!') {
    return WildcardOperatorKind::kInequality;
  }

  return WildcardOperatorKind::kUnknown;
}

namespace {

auto WildcardBadOp(WildcardOperatorKind kind) -> std::string_view {
  return kind == WildcardOperatorKind::kEquality ? "=?=" : "!?=";
}

}  // namespace

auto CheckWildcardOperatorFixableImpl(const SL::FileContent* fileContent,
                                      SL::ErrorContainer* errors,
                                      SL::SymbolTable* symbols,
                                      WildcardOperatorKind targetKind,
                                      SL::ErrorDefinition::ErrorType ruleId,
                                      WildcardOpStrings strings)
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
    if (DetectWildcardOperatorKind(fileContent, kWildEq) != targetKind) {
      continue;
    }

    const std::string_view symName = GetWildcardLhsName(fileContent, kWildEq);
    ReportError(fileContent, kWildEq, symName, ruleId, errors, symbols);

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
    d.message = std::string(strings.message);

    const std::string fix_desc =
        "replace '" + std::string(WildcardBadOp(targetKind)) + "' with '" +
        std::string(strings.correctOp) + "'";
    try {
      d.fixes.push_back(
          FixIt::Replace(op_range, std::string(strings.correctOp), fix_desc));
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
