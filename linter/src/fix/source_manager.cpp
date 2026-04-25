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

auto FixSourceManager::rangeLength(
    SURELOG::PathId fileId, unsigned bl, unsigned bc, unsigned el,
    unsigned ec) const  // NOLINT(bugprone-easily-swappable-parameters)
    -> unsigned {
  const unsigned start = getOffset(fileId, bl, bc);
  const unsigned end = getOffset(fileId, el, ec);
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

auto FixSourceManager::rangeLength(
    const std::string& filepath, unsigned bl, unsigned bc, unsigned el,
    unsigned ec) const  // NOLINT(bugprone-easily-swappable-parameters)
    -> unsigned {
  const auto it = path_index_.find(filepath);
  if (it == path_index_.end()) {
    return 0U;
  }
  return rangeLength(it->second, bl, bc, el, ec);
}

auto FixSourceManager::getLine(const std::string& filepath, unsigned line) const
    -> std::string {
  const auto it = path_index_.find(filepath);
  if (it == path_index_.end()) {
    return "";
  }
  return getLine(it->second, line);
}

auto FixSourceManager::getSlice(
    const std::string& filepath,
    unsigned line,  // NOLINT(bugprone-easily-swappable-parameters)
    unsigned col_begin, unsigned col_end) const -> std::string {
  const std::string full_line = getLine(filepath, line);
  if (col_begin >= full_line.size()) {
    return "";
  }
  const unsigned safe_end =
      std::min(col_end, static_cast<unsigned>(full_line.size()));
  return full_line.substr(col_begin, safe_end - col_begin);
}