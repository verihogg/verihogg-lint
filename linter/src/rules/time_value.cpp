#include "rules/time_value.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <iostream>
#include <string>
#include <vector>

#include "fix/fix_it.h"
#include "main/lint_diagnostics.h"
#include "main/lint_rules.h"
#include "utils/location_utils.h"

namespace SL = SURELOG;

namespace {

void AddWhitespaceFix(const std::string& filepath, LineCol diag_pos,
                      unsigned ws_begin_col, unsigned ws_end_col,
                      std::string_view number_sym, std::string_view unit_sym,
                      std::vector<LintDiagnostic>& diags) {
  LintDiagnostic d;
  d.filepath = filepath;
  d.line = diag_pos.line;
  d.col = diag_pos.col;
  d.message = std::string(number_sym) + " " + std::string(unit_sym) + " -> " +
              std::string(number_sym) + std::string(unit_sym);

  const FixRange ws_range{
      .begin = {.filename = filepath,
                .line = diag_pos.line,
                .col = ws_begin_col},
      .end = {.filename = filepath, .line = diag_pos.line, .col = ws_end_col},
  };

  try {
    d.fixes.push_back(FixIt::Remove(
        ws_range, "remove whitespace between number and time unit"));
  } catch (const std::exception& e) {
    std::cerr << "autofix: cannot create fix at " << filepath << ":"
              << diag_pos.line << ": " << e.what() << "\n";
    return;
  }

  diags.push_back(std::move(d));
}

void CheckTimeLiteralSiblings(const SL::FileContent* fileContent,
                              SL::NodeId timeLiteralId,
                              SL::ErrorContainer* errors,
                              SL::SymbolTable* symbols,
                              const std::string& filepath,
                              std::vector<LintDiagnostic>& diags) {
  SL::NodeId const kIntConst = fileContent->Child(timeLiteralId);
  if (!kIntConst ||
      fileContent->Type(kIntConst) != SL::VObjectType::slIntConst) {
    return;
  }

  SL::NodeId const kTimeUnit = fileContent->Sibling(kIntConst);
  if (!kTimeUnit ||
      fileContent->Type(kTimeUnit) != SL::VObjectType::paTime_unit) {
    return;
  }

  const auto kEndOfNumber = fileContent->EndColumn(kIntConst);
  const auto kStartOfUnit = fileContent->Column(kTimeUnit);
  if (kStartOfUnit <= kEndOfNumber) {
    return;
  }

  const auto kNumber = fileContent->SymName(kIntConst);
  const auto kUnit = fileContent->SymName(kTimeUnit);

  std::string badValue = std::string(kNumber) + " " + std::string(kUnit);
  ReportError(fileContent, kIntConst, badValue, verihogg_lint::LINT_TIME_VALUE,
              errors, symbols);

  const unsigned line = fileContent->Line(kIntConst);
  if (line == 0) {
    return;
  }

  AddWhitespaceFix(
      filepath,
      LineCol{.line = line,
              .col = static_cast<unsigned>(fileContent->Column(kIntConst))},
      static_cast<unsigned>(kEndOfNumber), static_cast<unsigned>(kStartOfUnit),
      kNumber, kUnit, diags);
}

void CheckTimeLiteralChild(const SL::FileContent* fileContent,
                           SL::NodeId intConstId, SL::ErrorContainer* errors,
                           SL::SymbolTable* symbols, FixSourceManager& sm,
                           const std::string& filepath,
                           std::vector<LintDiagnostic>& diags) {
  if (fileContent->Type(intConstId) != SL::VObjectType::slIntConst) {
    return;
  }

  SL::NodeId const kTimeUnit = fileContent->Child(intConstId);
  if (!kTimeUnit ||
      fileContent->Type(kTimeUnit) != SL::VObjectType::paTime_unit) {
    return;
  }

  const unsigned line = fileContent->Line(intConstId);
  if (line == 0) {
    return;
  }

  const std::string line_str = sm.getLine(filepath, line);
  if (line_str.empty()) {
    return;
  }

  const unsigned unit_col_0 = fileContent->Column(kTimeUnit) - 1;  // 0-based
  if (unit_col_0 == 0 || unit_col_0 > line_str.size()) {
    return;
  }

  unsigned end_num_0 = unit_col_0;
  while (end_num_0 > 0 && (line_str.at(end_num_0 - 1) == ' ' ||
                           line_str.at(end_num_0 - 1) == '\t')) {
    --end_num_0;
  }

  if (end_num_0 == unit_col_0) {
    return;
  }

  const auto kNumber = fileContent->SymName(intConstId);
  const auto kUnit = fileContent->SymName(kTimeUnit);

  std::string badValue = std::string(kNumber) + " " + std::string(kUnit);
  ReportError(fileContent, intConstId, badValue, verihogg_lint::LINT_TIME_VALUE,
              errors, symbols);

  AddWhitespaceFix(
      filepath,
      LineCol{.line = line,
              .col = static_cast<unsigned>(fileContent->Column(intConstId))},
      end_num_0 + 1, unit_col_0 + 1, kNumber, kUnit, diags);
}

}  // namespace

auto CheckTimeValueFixable(const SL::FileContent* fileContent,
                           SL::ErrorContainer* errors, SL::SymbolTable* symbols,
                           FixSourceManager& sm)
    -> std::vector<LintDiagnostic> {
  std::vector<LintDiagnostic> diags;

  if (fileContent == nullptr || errors == nullptr || symbols == nullptr) {
    return diags;
  }

  SL::NodeId const kRoot = fileContent->getRootNode();
  if (!kRoot) {
    return diags;
  }

  const std::string filepath = GetFixFilepath(fileContent);
  if (filepath.empty()) {
    return diags;
  }

  for (SL::NodeId const kTL :
       fileContent->sl_collect_all(kRoot, SL::VObjectType::paTime_literal)) {
    CheckTimeLiteralSiblings(fileContent, kTL, errors, symbols, filepath,
                             diags);
  }

  if (!sm.isLoaded(filepath)) {
    (void)sm.loadFile(fileContent->getFileId());
  }
  if (sm.isLoaded(filepath)) {
    for (SL::NodeId const kIC :
         fileContent->sl_collect_all(kRoot, SL::VObjectType::slIntConst)) {
      CheckTimeLiteralChild(fileContent, kIC, errors, symbols, sm, filepath,
                            diags);
    }
  }

  return diags;
}
