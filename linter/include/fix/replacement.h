#pragma once

#include <map>
#include <string>
#include <vector>

#include "fix/fix_it.h"
#include "fix/source_manager.h"

// Source edit stored as byte offset + length + replacement text.
struct Replacement {
  std::string filename;
  unsigned offset = 0;
  unsigned length = 0;
  std::string text;
  std::string rule_id;

  // Sort descending by offset so edits are applied end-to-start.
  bool operator<(const Replacement& o) const {
    if (offset != o.offset) {
      return offset > o.offset;
    }
    return length > o.length;
  }
};

// Convert FixIt line/col coordinates to byte-offset Replacement.
Replacement fixItToReplacement(const FixIt& fix, FixSourceManager& sm);

// Set of compatible replacements for a single file.
class Replacements {
 public:
  // Add replacement; return false and fill conflict_msg on overlap
  // (first-wins).
  bool add(const Replacement& r, std::string* conflict_msg = nullptr);

  // Apply all replacements to source string end-to-start; return patched
  // string.
  [[nodiscard]] std::string apply(const std::string& source) const;

  [[nodiscard]] bool empty() const { return repls_.empty(); }
  [[nodiscard]] size_t size() const { return repls_.size(); }

  auto begin() const { return repls_.cbegin(); }
  auto end() const { return repls_.cend(); }

 private:
  std::vector<Replacement> repls_;

  // Return true if two replacements have overlapping byte ranges.
  static bool overlaps(const Replacement& a, const Replacement& b);
};

// Aggregates per-file Replacements for the whole design.
class FileReplacements {
 public:
  // Add replacement to the appropriate file bucket.
  bool add(const Replacement& r, std::string* conflict_msg = nullptr);

  // Read file, apply replacements in memory, write atomically via tmp+rename.
  bool applyToFile(const std::string& filepath) const;

  // Apply replacements to every file; populate fixed/failed lists.
  void applyAll(std::vector<std::string>* fixed = nullptr,
                std::vector<std::string>* failed = nullptr) const;

  // Serialize all replacements to a YAML file via yaml-cpp.
  bool exportToYaml(const std::string& output_path) const;

  [[nodiscard]] bool empty() const { return by_file_.empty(); }

 private:
  std::map<std::string, Replacements> by_file_;
};
