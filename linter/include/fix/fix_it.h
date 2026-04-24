#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>

struct FixLocation {
  std::string filename;
  unsigned line = 0;
  unsigned col = 0;
  [[nodiscard]] auto valid() const noexcept -> bool {
    return !filename.empty() && line > 0 && col > 0;
  }
};

struct FixRange {
  FixLocation begin;
  FixLocation end;

  void validate() const {
    if (!begin.valid()) {
      throw std::invalid_argument("FixRange: begin is invalid");
    }
    if (!end.valid()) {
      throw std::invalid_argument("FixRange: end is invalid");
    }
    if (begin.filename != end.filename) {
      throw std::invalid_argument(
          "FixRange: cross-file range not supported; begin='" + begin.filename +
          "' end='" + end.filename + "'");
    }
    if (begin.line > end.line ||
        (begin.line == end.line && begin.col > end.col)) {
      throw std::invalid_argument(
          "FixRange: end (" + std::to_string(end.line) + ":" +
          std::to_string(end.col) + ") is before begin (" +
          std::to_string(begin.line) + ":" + std::to_string(begin.col) + ")");
    }
  }
};

enum class FixKind : std::uint8_t { Insertion, Replacement, Removal };

struct FixIt {
  FixKind kind;
  FixRange range;
  std::string replacement;
  std::string description;

  static auto Insert(const FixLocation& pos, std::string text, std::string desc)
      -> FixIt {
    if (!pos.valid()) {
      throw std::invalid_argument("FixIt::Insert: invalid location");
    }
    if (desc.empty()) {
      throw std::invalid_argument("FixIt::Insert: description is empty");
    }
    return FixIt{.kind = FixKind::Insertion,
                 .range = FixRange{.begin = pos, .end = pos},
                 .replacement = std::move(text),
                 .description = std::move(desc)};
  }
  static auto Replace(const FixRange& range, std::string text, std::string desc)
      -> FixIt {
    range.validate();
    if (desc.empty()) {
      throw std::invalid_argument("FixIt::Replace: description is empty");
    }
    return FixIt{.kind = FixKind::Replacement,
                 .range = range,
                 .replacement = std::move(text),
                 .description = std::move(desc)};
  }
  static auto Remove(const FixRange& range, std::string desc) -> FixIt {
    range.validate();
    if (desc.empty()) {
      throw std::invalid_argument("FixIt::Remove: description is empty");
    }
    return FixIt{.kind = FixKind::Removal,
                 .range = range,
                 .replacement = "",
                 .description = std::move(desc)};
  }
};