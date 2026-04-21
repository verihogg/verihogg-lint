#pragma once

#include <string>

// Source location: filename, 1-based line and column.
struct FixLocation {
  std::string filename;
  unsigned line = 0;
  unsigned col = 0;
};

// Half-open byte range [begin, end) in source file.
struct FixRange {
  FixLocation begin;
  FixLocation end;
};

enum class FixKind {
  Insertion,    // insert text at a point (no removal)
  Replacement,  // replace range with new text
  Removal,      // remove range (empty replacement)
};

// Atomic source edit: kind, range, new text, and human-readable description.
struct FixIt {
  FixKind kind;
  FixRange range;
  std::string replacement;
  std::string description;

  // Insert text before pos without removing anything (begin == end).
  static FixIt Insert(FixLocation pos, std::string text,
                      std::string desc = "") {
    FixRange r;
    r.begin = pos;
    r.end = pos;
    return FixIt{FixKind::Insertion, r, std::move(text), std::move(desc)};
  }

  // Replace range [begin, end) with text.
  static FixIt Replace(FixRange range, std::string text,
                       std::string desc = "") {
    return FixIt{FixKind::Replacement, range, std::move(text), std::move(desc)};
  }

  // Remove range [begin, end).
  static FixIt Remove(FixRange range, std::string desc = "") {
    return FixIt{FixKind::Removal, range, "", std::move(desc)};
  }
};
