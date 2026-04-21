#pragma once

#include <Surelog/Common/FileSystem.h>
#include <Surelog/Common/PathId.h>

#include <string>
#include <unordered_map>
#include <vector>

// Converts (line, col) to byte offset by reading files via Surelog FileSystem.
class FixSourceManager {
 public:
  // Load file via Surelog FileSystem, build offset table, cache by PathId and
  // string path.
  bool loadFile(SURELOG::PathId fileId);

  // Return byte offset of (line, col); throws out_of_range if not loaded or out
  // of bounds.
  [[nodiscard]] unsigned getOffset(SURELOG::PathId fileId, unsigned line,
                                   unsigned col) const;

  // Return byte length of range [begin, end).
  [[nodiscard]] unsigned rangeLength(SURELOG::PathId fileId,
                                     unsigned begin_line, unsigned begin_col,
                                     unsigned end_line, unsigned end_col) const;

  // Return line content (without newline) by 1-based line number.
  [[nodiscard]] std::string getLine(SURELOG::PathId fileId,
                                    unsigned line) const;

  // Return true if the file identified by PathId is loaded in cache.
  [[nodiscard]] bool isLoaded(SURELOG::PathId fileId) const {
    return cache_.count(fileId) > 0;
  }

  // Look up filepath in path_index_, delegate to PathId overload.
  [[nodiscard]] unsigned getOffset(const std::string& filepath, unsigned line,
                                   unsigned col) const;

  // Delegate rangeLength through path_index_; return 0 if not found.
  [[nodiscard]] unsigned rangeLength(const std::string& filepath,
                                     unsigned begin_line, unsigned begin_col,
                                     unsigned end_line, unsigned end_col) const;

  // Delegate getLine through path_index_; return "" if not found.
  [[nodiscard]] std::string getLine(const std::string& filepath,
                                    unsigned line) const;

  // Return true if the file identified by string path is loaded in cache.
  [[nodiscard]] bool isLoaded(const std::string& filepath) const {
    return path_index_.count(filepath) > 0;
  }

 private:
  struct FileData {
    std::string content;
    // line_offsets[i] = byte offset of line (i+1); line_offsets[0] = 0.
    std::vector<unsigned> line_offsets;
  };

  // Build line_offsets table from raw file content.
  static FileData buildFileData(const std::string& content);

  std::unordered_map<SURELOG::PathId, FileData, SURELOG::PathIdHasher,
                     SURELOG::PathIdEqualityComparer>
      cache_;

  // Secondary index: string path → PathId, populated in loadFile().
  std::unordered_map<std::string, SURELOG::PathId> path_index_;
};
