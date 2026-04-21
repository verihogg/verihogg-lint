#include "fix/source_manager.h"

#include <stdexcept>
#include <string>

// Load file via Surelog FileSystem, build offset table, cache by PathId and
// string path.
bool FixSourceManager::loadFile(SURELOG::PathId fileId) {
  // TODO
  (void)fileId;
  return false;
}

// Build line_offsets table: line_offsets[i] = byte offset of line (i+1).
FixSourceManager::FileData FixSourceManager::buildFileData(
    const std::string& content) {
  // TODO
  (void)content;
  return FileData{};
}

// Return byte offset of (line, col); throws out_of_range if not loaded or out
// of bounds.
unsigned FixSourceManager::getOffset(SURELOG::PathId fileId, unsigned line,
                                     unsigned col) const {
  // TODO
  (void)fileId;
  (void)line;
  (void)col;
  throw std::out_of_range("FixSourceManager::getOffset — not implemented");
}

// Return byte length of range [begin, end).
unsigned FixSourceManager::rangeLength(SURELOG::PathId fileId,
                                       unsigned begin_line, unsigned begin_col,
                                       unsigned end_line,
                                       unsigned end_col) const {
  // TODO
  (void)fileId;
  (void)begin_line;
  (void)begin_col;
  (void)end_line;
  (void)end_col;
  return 0;
}

// Return line content (without newline) by 1-based line number.
std::string FixSourceManager::getLine(SURELOG::PathId fileId,
                                      unsigned line) const {
  // TODO
  (void)fileId;
  (void)line;
  return "";
}

// Look up filepath in path_index_, delegate to PathId overload.
unsigned FixSourceManager::getOffset(const std::string& filepath, unsigned line,
                                     unsigned col) const {
  // TODO
  (void)filepath;
  (void)line;
  (void)col;
  throw std::out_of_range(
      "FixSourceManager::getOffset(string) — not implemented");
}

// Delegate rangeLength through path_index_; return 0 if not found.
unsigned FixSourceManager::rangeLength(const std::string& filepath,
                                       unsigned begin_line, unsigned begin_col,
                                       unsigned end_line,
                                       unsigned end_col) const {
  // TODO
  (void)filepath;
  (void)begin_line;
  (void)begin_col;
  (void)end_line;
  (void)end_col;
  return 0;
}

// Delegate getLine through path_index_; return "" if not found.
std::string FixSourceManager::getLine(const std::string& filepath,
                                      unsigned line) const {
  // TODO
  (void)filepath;
  (void)line;
  return "";
}
