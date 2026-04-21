#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "fix/fix_it.h"

// Single lint violation with attached autofix hints.
struct LintDiagnostic {
  std::string rule_id;
  std::string filepath;
  unsigned line = 0;
  unsigned col = 0;
  std::vector<FixIt> fixes;
};

// Accumulates LintDiagnostic instances produced by fixable rules.
class LintDiagnosticCollector {
 public:
  // Append a diagnostic.
  void add(LintDiagnostic diag) { diags_.push_back(std::move(diag)); }

  // Return all collected diagnostics.
  const std::vector<LintDiagnostic>& all() const { return diags_; }

  // Count diagnostics that have at least one FixIt.
  [[nodiscard]] std::size_t fixableCount() const {
    std::size_t n = 0;
    for (const auto& d : diags_) {
      if (!d.fixes.empty()) {
        ++n;
      }
    }
    return n;
  }

  // Return true if any fixable diagnostic exists.
  [[nodiscard]] bool hasFixable() const { return fixableCount() > 0; }

 private:
  std::vector<LintDiagnostic> diags_;
};
