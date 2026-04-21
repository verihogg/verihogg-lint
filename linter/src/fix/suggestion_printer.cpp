#include "fix/suggestion_printer.h"

// Print fix-available notes for all fixable diagnostics, then show --fix hint.
void SuggestionPrinter::print(const std::vector<LintDiagnostic>& diags,
                              FixSourceManager* source_mgr, bool show_diff) {
  // TODO
  (void)diags;
  (void)source_mgr;
  (void)show_diff;
}

// Print note with location, description, and optional before/after diff lines.
void SuggestionPrinter::printOneDiag(const LintDiagnostic& d,
                                     FixSourceManager* source_mgr,
                                     bool show_diff) {
  // TODO
  (void)d;
  (void)source_mgr;
  (void)show_diff;
}
