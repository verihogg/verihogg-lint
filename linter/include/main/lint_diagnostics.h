#pragma once

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
    std::size_t n = 0;
    for (const auto& d : diags_) {
      if (!d.fixes.empty()) {
        ++n;
      }
    }
    return n;
  }

  [[nodiscard]] auto hasFixable() const -> bool {
    for (const auto& d : diags_) {
      if (!d.fixes.empty()) {
        return true;
      }
    }
    return false;
  }

 private:
  std::vector<LintDiagnostic> diags_;
};