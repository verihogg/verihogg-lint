#include "fix/source_manager.h"

#include <Surelog/Common/FileSystem.h>

#include <stdexcept>
#include <string>

auto FixSourceManager::loadFile(SURELOG::PathId fileId) -> bool {
  if (cache_.count(fileId) > 0) {
    return true;
  }

  std::string content;
  if (!SURELOG::FileSystem::getInstance()->readContent(fileId, content)) {
    return false;
  }

  const std::string_view pathView =
      SURELOG::FileSystem::getInstance()->toPath(fileId);
  if (!pathView.empty()) {
    path_index_[std::string(pathView)] = fileId;
  }

  cache_[fileId] = buildFileData(std::move(content));
  return true;
}

auto FixSourceManager::buildFileData(std::string content)
    -> FixSourceManager::FileData {
  FileData fd;
  fd.content = std::move(content);
  fd.line_offsets.push_back(0U);

  for (uint32_t i = 0; i < static_cast<uint32_t>(fd.content.size()); ++i) {
    if (fd.content.at(i) == '\n') {
      fd.line_offsets.push_back(i + 1U);
    }
  }
  return fd;
}

auto FixSourceManager::getOffset(SURELOG::PathId fileId, unsigned line,
                                 unsigned col) const -> unsigned {
  const auto it = cache_.find(fileId);
  if (it == cache_.end()) {
    throw std::out_of_range(
        "FixSourceManager: file not loaded; call loadFile() before "
        "getOffset()");
  }
  const FileData& fd = it->second;

  if (line == 0 || line > static_cast<unsigned>(fd.line_offsets.size())) {
    throw std::out_of_range("FixSourceManager: line " + std::to_string(line) +
                            " out of range (file has " +
                            std::to_string(fd.line_offsets.size()) + " lines)");
  }

  const unsigned line_start = fd.line_offsets.at(line - 1U);
  const unsigned col_offset = (col > 0U) ? (col - 1U) : 0U;
  const unsigned offset = line_start + col_offset;

  if (offset > static_cast<unsigned>(fd.content.size())) {
    throw std::out_of_range("FixSourceManager: col " + std::to_string(col) +
                            " out of range at line " + std::to_string(line));
  }
  return offset;
}

auto FixSourceManager::rangeLength(SURELOG::PathId fileId,
                                   LineRange range) const -> unsigned {
  const unsigned start = getOffset(fileId, range.begin.line, range.begin.col);
  const unsigned end = getOffset(fileId, range.end.line, range.end.col);
  return (end >= start) ? (end - start) : 0U;
}

auto FixSourceManager::getLine(SURELOG::PathId fileId, unsigned line) const
    -> std::string {
  const auto it = cache_.find(fileId);
  if (it == cache_.end()) {
    return "";
  }
  const FileData& fd = it->second;

  if (line == 0 || line > static_cast<unsigned>(fd.line_offsets.size())) {
    return "";
  }

  const unsigned start = fd.line_offsets.at(line - 1U);
  unsigned end = start;
  while (end < static_cast<unsigned>(fd.content.size()) &&
         fd.content.at(end) != '\n') {
    ++end;
  }

  if (end > start && fd.content.at(end - 1) == '\r') {
    --end;
  }

  return fd.content.substr(start, end - start);
}

auto FixSourceManager::getOffset(const std::string& filepath, unsigned line,
                                 unsigned col) const -> unsigned {
  const auto it = path_index_.find(filepath);
  if (it == path_index_.end()) {
    throw std::out_of_range("FixSourceManager: file not loaded: " + filepath);
  }
  return getOffset(it->second, line, col);
}

auto FixSourceManager::rangeLength(const std::string& filepath,
                                   LineRange range) const -> unsigned {
  const auto it = path_index_.find(filepath);
  if (it == path_index_.end()) {
    return 0U;
  }
  return rangeLength(it->second, range);
}

auto FixSourceManager::getLine(const std::string& filepath, unsigned line) const
    -> std::string {
  const auto it = path_index_.find(filepath);
  if (it == path_index_.end()) {
    return "";
  }
  return getLine(it->second, line);
}

auto FixSourceManager::getSlice(const std::string& filepath, LineCol begin,
                                unsigned col_end) const -> std::string {
  const auto path_it = path_index_.find(filepath);
  if (path_it == path_index_.end()) {
    return "";
  }
  const auto cache_it = cache_.find(path_it->second);
  if (cache_it == cache_.end()) {
    return "";
  }
  const FileData& fd = cache_it->second;

  if (begin.line == 0 ||
      begin.line > static_cast<unsigned>(fd.line_offsets.size())) {
    return "";
  }

  const unsigned line_start = fd.line_offsets.at(begin.line - 1);
  unsigned line_end = line_start;
  while (line_end < static_cast<unsigned>(fd.content.size()) &&
         fd.content.at(line_end) != '\n') {
    ++line_end;
  }
  if (line_end > line_start && fd.content.at(line_end - 1) == '\r') {
    --line_end;
  }

  const unsigned line_len = line_end - line_start;
  if (begin.col >= line_len) {
    return "";
  }
  const unsigned safe_end = std::min(col_end, line_len);
  return fd.content.substr(line_start + begin.col, safe_end - begin.col);
}