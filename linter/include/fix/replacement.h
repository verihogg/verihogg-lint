#pragma once

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

  bool operator<(const Replacement& o) const {
    if (offset != o.offset) {
      return offset > o.offset;
    }
    return length > o.length;
  }
};

Replacement fixItToReplacement(const FixIt& fix, FixSourceManager& sm);

class Replacements {
 public:
  [[nodiscard]] bool add(const Replacement& r,
                         std::string* conflict_msg = nullptr);

  [[nodiscard]] std::string apply(const std::string& source) const;

  [[nodiscard]] bool empty() const { return repls_.empty(); }
  [[nodiscard]] size_t size() const { return repls_.size(); }

  auto begin() const { return repls_.cbegin(); }
  auto end() const { return repls_.cend(); }
  auto rbegin() const { return repls_.crbegin(); }
  auto rend() const { return repls_.crend(); }

 private:
  std::vector<Replacement> repls_;

  static bool overlaps(const Replacement& a, const Replacement& b);
};

class FileReplacements {
 public:
  [[nodiscard]] bool add(const Replacement& r,
                         std::string* conflict_msg = nullptr);

  [[nodiscard]] bool applyToFile(const std::string& filepath,
                                 const std::string& backup_suffix = "") const;

  void applyAll(std::vector<std::string>* fixed = nullptr,
                std::vector<std::string>* failed = nullptr,
                const std::string& backup_suffix = "") const;

  void printDiff() const;

  [[nodiscard]] bool exportToYaml(const std::string& output_path) const;

  [[nodiscard]] bool empty() const { return by_file_.empty(); }

 private:
  std::map<std::string, Replacements> by_file_;
};
