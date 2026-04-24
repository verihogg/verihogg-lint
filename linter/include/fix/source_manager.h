#pragma once

#include <Surelog/Common/FileSystem.h>
#include <Surelog/Common/PathId.h>

#include <string>
#include <unordered_map>
#include <vector>

class FixSourceManager {
 public:
  [[nodiscard]] auto loadFile(SURELOG::PathId fileId) -> bool;

  [[nodiscard]] auto getOffset(SURELOG::PathId fileId, unsigned line,
                               unsigned col) const -> unsigned;
  [[nodiscard]] auto getOffset(const std::string& filepath, unsigned line,
                               unsigned col) const -> unsigned;

  [[nodiscard]] auto rangeLength(SURELOG::PathId fileId, unsigned bl,
                                 unsigned bc, unsigned el, unsigned ec) const
      -> unsigned;
  [[nodiscard]] auto rangeLength(const std::string& filepath, unsigned bl,
                                 unsigned bc, unsigned el, unsigned ec) const
      -> unsigned;

  [[nodiscard]] auto getLine(SURELOG::PathId fileId, unsigned line) const
      -> std::string;
  [[nodiscard]] auto getLine(const std::string& filepath, unsigned line) const
      -> std::string;

  [[nodiscard]] auto getSlice(const std::string& filepath, unsigned line,
                              unsigned col_begin, unsigned col_end) const
      -> std::string;

  [[nodiscard]] auto isLoaded(SURELOG::PathId fileId) const -> bool {
    return cache_.count(fileId) > 0;
  }
  [[nodiscard]] auto isLoaded(const std::string& filepath) const -> bool {
    return path_index_.count(filepath) > 0;
  }

 private:
  struct FileData {
    std::string content;
    std::vector<uint32_t> line_offsets;
  };

  static auto buildFileData(std::string content) -> FileData;

  std::unordered_map<SURELOG::PathId, FileData, SURELOG::PathIdHasher,
                     SURELOG::PathIdEqualityComparer>
      cache_;

  std::unordered_map<std::string, SURELOG::PathId> path_index_;
};