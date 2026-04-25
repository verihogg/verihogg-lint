#include "fix/suggestion_printer.h"

#include <iostream>

void SuggestionPrinter::print(const std::vector<LintDiagnostic>& diags,
                              FixSourceManager* source_mgr, bool show_diff) {
  bool any_fixable = false;
  for (const auto& d : diags) {
    if (!d.fixes.empty()) {
      any_fixable = true;
      break;
    }
  }
  if (!any_fixable) {
    return;
  }

  std::cerr << "\n";
  for (const auto& d : diags) {
    if (!d.fixes.empty()) {
      printOneDiag(d, source_mgr, show_diff);
    }
  }
  std::cerr << "Run with --fix to apply all suggestions automatically.\n";
}

void SuggestionPrinter::printOneDiag(const LintDiagnostic& d,
                                     FixSourceManager* source_mgr,
                                     bool show_diff) {
  std::cerr << d.filepath << ":" << d.line << ":" << d.col
            << ": note: fix available [" << d.rule_id << "]\n";

  for (const auto& fix : d.fixes) {
    std::cerr << "  suggestion: " << fix.description << "\n";

    if (!show_diff || source_mgr == nullptr) {
      continue;
    }
    if (!source_mgr->isLoaded(d.filepath)) {
      continue;
    }

    const std::string before_line =
        source_mgr->getLine(d.filepath, fix.range.begin.line);
    if (before_line.empty()) {
      continue;
    }

    std::cerr << "  before: " << before_line << "\n";

    const unsigned col0 =
        (fix.range.begin.col > 0U) ? (fix.range.begin.col - 1U) : 0U;

    std::string after_line;

    if (fix.kind == FixKind::Insertion) {
      if (col0 <= before_line.size()) {
        after_line = before_line.substr(0, col0);
        after_line += fix.replacement;
        after_line += before_line.substr(col0);
      }
    } else if (fix.range.begin.line == fix.range.end.line) {
      const unsigned end0 =
          (fix.range.end.col > 0U) ? (fix.range.end.col - 1U) : 0U;
      if (col0 <= before_line.size() && end0 <= before_line.size() &&
          col0 <= end0) {
        after_line = before_line.substr(0, col0);
        after_line += fix.replacement;
        after_line += before_line.substr(end0);
      }
    } else {
      after_line = "(multiline fix)";
    }

    if (!after_line.empty()) {
      std::cerr << "  after:  " << after_line << "\n";
    }
  }
}
