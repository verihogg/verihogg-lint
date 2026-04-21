#include "fix/replacement.h"

#include <string>

// Convert FixIt line/col coordinates to byte-offset Replacement via
// FixSourceManager.
Replacement fixItToReplacement(const FixIt& fix, FixSourceManager& sm) {
  // TODO
  (void)fix;
  (void)sm;
  return Replacement{};
}

// Return true if two replacements have overlapping byte ranges.
bool Replacements::overlaps(const Replacement& a, const Replacement& b) {
  // TODO
  (void)a;
  (void)b;
  return false;
}

// Add replacement; reject with conflict_msg on overlap (first-wins strategy).
bool Replacements::add(const Replacement& r, std::string* conflict_msg) {
  // TODO
  (void)conflict_msg;
  repls_.push_back(r);
  return true;
}

// Apply all replacements to source string end-to-start; return patched string.
std::string Replacements::apply(const std::string& source) const {
  // TODO
  return source;
}

bool FileReplacements::add(const Replacement& r, std::string* conflict_msg) {
  return by_file_[r.filename].add(r, conflict_msg);
}

// Read file, apply replacements in memory, write result atomically via
// tmp+rename.
bool FileReplacements::applyToFile(const std::string& filepath) const {
  // TODO
  (void)filepath;
  return false;
}

// Apply replacements to every file; populate fixed/failed lists.
void FileReplacements::applyAll(std::vector<std::string>* fixed,
                                std::vector<std::string>* failed) const {
  // TODO
  (void)fixed;
  (void)failed;
}

// Serialize all replacements to a YAML file via yaml-cpp.
bool FileReplacements::exportToYaml(const std::string& output_path) const {
  // TODO
  (void)output_path;
  return false;
}
