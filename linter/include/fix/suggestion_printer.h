#pragma once

#include <vector>

#include "fix/source_manager.h"
#include "main/lint_diagnostics.h"

class SuggestionPrinter {
 public:
  static void print(const std::vector<LintDiagnostic>& diags,
                    FixSourceManager* source_mgr, bool show_diff = true);

 private:
  static void printOneDiag(const LintDiagnostic& d,
                           FixSourceManager* source_mgr, bool show_diff);
};
