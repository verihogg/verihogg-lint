#include "rules/exponent_format_time_value.h"

#include <Surelog/Common/FileSystem.h>
#include <Surelog/Common/NodeId.h>
#include <Surelog/Common/PathId.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/ErrorReporting/ErrorDefinition.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include "utils/location_utils.h"

namespace SL = SURELOG;

using LineCache = std::vector<std::string>;

namespace {
auto GetLines(SL::PathId fileId) -> const LineCache& {
  static std::unordered_map<uint32_t, LineCache> sFileCache;
  const auto kEy = static_cast<uint32_t>(fileId);
  auto cacheIt = sFileCache.find(kEy);
  if (cacheIt != sFileCache.end()) {
    return cacheIt->second;
  }

  LineCache& lines = sFileCache[kEy];

  SL::FileSystem* fileSystem = SL::FileSystem::getInstance();
  if (fileSystem == nullptr) {
    return lines;
  }

  const std::string kPath = std::string(fileSystem->toPath(fileId));
  if (kPath.empty()) {
    return lines;
  }

  std::ifstream ifs(kPath);
  if (!ifs.is_open()) {
    return lines;
  }

  std::string line;
  while (std::getline(ifs, line)) {
    lines.push_back(std::move(line));
  }
  return lines;
}
auto GetTokenText(SL::PathId fileId, uint32_t line, uint32_t colStart,
                  uint32_t colEnd) -> std::string {
  if (line == 0 || colStart == 0 || colEnd <= colStart) {
    return {};
  }

  const LineCache& lines = GetLines(fileId);
  if (line > lines.size()) {
    return {};
  }

  const std::string& srcLine = lines[line - 1];

  const auto kStart = static_cast<size_t>(colStart - 1);
  const auto kEnd = static_cast<size_t>(colEnd - 1);

  if (kStart >= srcLine.size()) {
    return {};
  }
  const size_t kLen = std::min(kEnd, srcLine.size()) - kStart;

  return srcLine.substr(kStart, kLen);
}

auto TokenContainsExponent(const SL::FileContent* fileContent,
                           SL::NodeId numNode) -> bool {
  const SL::PathId kFileId = fileContent->getFileId(numNode);
  const uint32_t kLine = fileContent->Line(numNode);
  const uint32_t kColStart = fileContent->Column(numNode);
  const uint32_t kColEnd = fileContent->EndColumn(numNode);

  const std::string kText = GetTokenText(kFileId, kLine, kColStart, kColEnd);
  if (kText.empty()) {
    return false;
  }

  return std::ranges::any_of(kText,
                             [](char chr) { return chr == 'e' || chr == 'E'; });
}
void CheckTimeLiteralForExponent(const SL::FileContent* fileContent,
                                 SL::NodeId timeLiteral,
                                 SL::ErrorContainer* errors,
                                 SL::SymbolTable* symbols) {
  SL::NodeId const numNode = fileContent->Child(timeLiteral);
  if (!numNode) {
    return;
  }

  const SL::VObjectType kNumType = fileContent->Type(numNode);
  if (kNumType != SL::VObjectType::slIntConst &&
      kNumType != SL::VObjectType::slRealConst) {
    {
      return;
    }
  }

  SL::NodeId const timeUnitNode = fileContent->Sibling(numNode);
  if (!timeUnitNode) {
    return;
  }
  if (fileContent->Type(timeUnitNode) != SL::VObjectType::paTime_unit) {
    return;
  }

  if (!TokenContainsExponent(fileContent, numNode)) {
    return;
  }

  const SL::PathId kFileId = fileContent->getFileId(numNode);
  const uint32_t kLine = fileContent->Line(numNode);
  const uint32_t kColStart = fileContent->Column(numNode);
  const uint32_t kColEnd = fileContent->EndColumn(numNode);
  const std::string_view kUnit = fileContent->SymName(timeUnitNode);

  std::string originalNum = GetTokenText(kFileId, kLine, kColStart, kColEnd);
  if (originalNum.empty()) {
    originalNum = fileContent->SymName(numNode);
  }

  std::string badValue;
  badValue.reserve(originalNum.size() + kUnit.size());
  badValue.append(originalNum).append(kUnit);

  ReportError(fileContent, numNode, badValue,
              SL::ErrorDefinition::LINT_EXPONENT_FORMAT_TIME_VALUE, errors,
              symbols);
}
}  // namespace

void CheckExponentFormatTimeValue(const SL::FileContent* fileContent,
                                  SL::ErrorContainer* errors,
                                  SL::SymbolTable* symbols) {
  if (fileContent == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }

  SL::NodeId const root = fileContent->getRootNode();
  if (!root) {
    return;
  }

  for (SL::NodeId const timeLiteral :
       fileContent->sl_collect_all(root, SL::VObjectType::paTime_literal)) {
    CheckTimeLiteralForExponent(fileContent, timeLiteral, errors, symbols);
  }
}