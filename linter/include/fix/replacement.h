#pragma once

#include <compare>
#include <map>
#include <string>
#include <vector>

#include "fix/fix_it.h"
#include "fix/source_manager.h"

struct Replacement {
  std::string filename;
  unsigned offset = 0;
  unsigned length = 0;
  std::string text;
  std::string rule_id;

  auto operator<=>(const Replacement& o) const -> std::weak_ordering {
    if (offset != o.offset) {
      return o.offset <=> offset;  // descending by offset
    }
    return o.length <=> length;  // descending by length
  }
  auto operator==(const Replacement& o) const -> bool = default;
};

auto fixItToReplacement(const FixIt& fix, FixSourceManager& sm) -> Replacement;

class Replacements {
 public:
  [[nodiscard]] auto add(const Replacement& r,
                         std::string* conflict_msg = nullptr) -> bool;

  [[nodiscard]] auto apply(const std::string& source) const -> std::string;

  [[nodiscard]] auto empty() const -> bool { return repls_.empty(); }
  [[nodiscard]] auto size() const -> size_t { return repls_.size(); }

  [[nodiscard]] auto begin() const { return repls_.cbegin(); }
  [[nodiscard]] auto end() const { return repls_.cend(); }
  [[nodiscard]] auto rbegin() const { return repls_.crbegin(); }
  [[nodiscard]] auto rend() const { return repls_.crend(); }

 private:
  std::vector<Replacement> repls_;

  static auto overlaps(const Replacement& a, const Replacement& b) -> bool;
};

class FileReplacements {
 public:
  [[nodiscard]] auto add(const Replacement& r,
                         std::string* conflict_msg = nullptr) -> bool;

  [[nodiscard]] auto applyToFile(const std::string& filepath,
                                 const std::string& backup_suffix = "") const
      -> bool;

  void applyAll(std::vector<std::string>* fixed = nullptr,
                std::vector<std::string>* failed = nullptr,
                const std::string& backup_suffix = "") const;

  void printDiff() const;

  [[nodiscard]] auto exportToYaml(const std::string& output_path) const -> bool;

  [[nodiscard]] auto empty() const -> bool { return by_file_.empty(); }

 private:
  std::map<std::string, Replacements> by_file_;
};
