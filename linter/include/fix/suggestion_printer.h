#pragma once

#include <vector>

#include "fix/source_manager.h"
#include "main/lint_diagnostics.h"

// Prints "note: fix available" hints to stderr after errors->printMessages().
class SuggestionPrinter {
 public:
  // Print fix-available notes for all fixable diagnostics, then show --fix
  // hint.
  static void print(const std::vector<LintDiagnostic>& diags,
                    FixSourceManager* source_mgr, bool show_diff = true);

 private:
  // Print note with location, description, and optional before/after diff
  // lines.
  static void printOneDiag(const LintDiagnostic& d,
                           FixSourceManager* source_mgr, bool show_diff);
};
