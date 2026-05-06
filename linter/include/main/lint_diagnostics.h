#pragma once

#include <algorithm>
#include <cstddef>
#include <string>
#include <vector>

#include "fix/fix_it.h"

struct LintDiagnostic {
  std::string rule_id;
  std::string filepath;
  unsigned line = 0;
  unsigned col = 0;
  std::string message;
  std::vector<FixIt> fixes;
};

class LintDiagnosticCollector {
 public:
  void add(LintDiagnostic diag) { diags_.push_back(std::move(diag)); }

  [[nodiscard]] auto all() const -> const std::vector<LintDiagnostic>& {
    return diags_;
  }

  [[nodiscard]] auto fixableCount() const -> std::size_t {
    return static_cast<std::size_t>(std::count_if(
        diags_.begin(), diags_.end(),
        [](const LintDiagnostic& d) { return !d.fixes.empty(); }));
  }

  [[nodiscard]] auto hasFixable() const -> bool {
    return std::ranges::any_of(
        diags_, [](const LintDiagnostic& d) { return !d.fixes.empty(); });
  }

 private:
  std::vector<LintDiagnostic> diags_;
};