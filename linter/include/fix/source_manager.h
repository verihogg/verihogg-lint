#pragma once

#include <Surelog/Common/FileSystem.h>
#include <Surelog/Common/PathId.h>

#include <string>
#include <unordered_map>
#include <vector>

struct LineCol {
  unsigned line = 0;
  unsigned col = 0;
};

struct LineRange {
  LineCol begin;
  LineCol end;
};

class FixSourceManager {
 public:
  [[nodiscard]] auto loadFile(SURELOG::PathId fileId) -> bool;

  [[nodiscard]] auto getOffset(SURELOG::PathId fileId, unsigned line,
                               unsigned col) const -> unsigned;
  [[nodiscard]] auto getOffset(const std::string& filepath, unsigned line,
                               unsigned col) const -> unsigned;

  [[nodiscard]] auto rangeLength(SURELOG::PathId fileId, LineRange range) const
      -> unsigned;
  [[nodiscard]] auto rangeLength(const std::string& filepath,
                                 LineRange range) const -> unsigned;

  [[nodiscard]] auto getLine(SURELOG::PathId fileId, unsigned line) const
      -> std::string;
  [[nodiscard]] auto getLine(const std::string& filepath, unsigned line) const
      -> std::string;

  [[nodiscard]] auto getSlice(const std::string& filepath, LineCol begin,
                              unsigned col_end) const -> std::string;

  [[nodiscard]] auto isLoaded(SURELOG::PathId fileId) const -> bool {
    return cache_.contains(fileId);
  }
  [[nodiscard]] auto isLoaded(const std::string& filepath) const -> bool {
    return path_index_.contains(filepath);
  }

 private:
  struct FileData {
    std::string content;
    std::vector<uint32_t> line_offsets;
  };

  static auto buildFileData(std::string content) -> FileData;
  // Returns the offset of the character past the last non-CR/LF content byte
  // on the given line (i.e. the stripped line end, not the '\n' itself).
  static auto findLineEnd(const FileData& fd, unsigned line_start) -> unsigned;

  std::unordered_map<SURELOG::PathId, FileData, SURELOG::PathIdHasher,
                     SURELOG::PathIdEqualityComparer>
      cache_;

  std::unordered_map<std::string, SURELOG::PathId> path_index_;
};
