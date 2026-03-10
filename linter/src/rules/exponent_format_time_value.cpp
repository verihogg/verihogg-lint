#include "rules/exponent_format_time_value.h"

#include <fstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "Surelog/Common/FileSystem.h"
#include "Surelog/Design/FileContent.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"
#include "Surelog/SourceCompile/VObjectTypes.h"
#include "utils/location_utils.h"

using namespace SURELOG;

using LineCache = std::vector<std::string>;
static std::unordered_map<uint32_t, LineCache> sFileCache;

static const LineCache& GetLines(PathId fileId) {
  const uint32_t key = static_cast<uint32_t>(fileId);
  auto it = sFileCache.find(key);
  if (it != sFileCache.end()) return it->second;

  LineCache& lines = sFileCache[key];

  FileSystem* fs = FileSystem::getInstance();
  if (!fs) return lines;

  const std::string path = std::string(fs->toPath(fileId));
  if (path.empty()) return lines;

  std::ifstream ifs(path);
  if (!ifs.is_open()) return lines;

  std::string line;
  while (std::getline(ifs, line)) {
    lines.push_back(std::move(line));
  }
  return lines;
}
static std::string GetTokenText(PathId fileId, uint32_t line, uint32_t colStart,
                                uint32_t colEnd) {
  if (line == 0 || colStart == 0 || colEnd <= colStart) return {};

  const LineCache& lines = GetLines(fileId);
  if (line > lines.size()) return {};

  const std::string& srcLine = lines[line - 1];

  const size_t start = static_cast<size_t>(colStart - 1);
  const size_t end = static_cast<size_t>(colEnd - 1);

  if (start >= srcLine.size()) return {};
  const size_t len = std::min(end, srcLine.size()) - start;

  return srcLine.substr(start, len);
}

static bool TokenContainsExponent(const FileContent* fC, NodeId numNode) {
  const PathId fileId = fC->getFileId(numNode);
  const uint32_t line = fC->Line(numNode);
  const uint32_t colStart = fC->Column(numNode);
  const uint32_t colEnd = fC->EndColumn(numNode);

  const std::string text = GetTokenText(fileId, line, colStart, colEnd);
  if (text.empty()) return false;

  for (char ch : text) {
    if (ch == 'e' || ch == 'E') return true;
  }
  return false;
}
static void CheckTimeLiteralForExponent(const FileContent* fC,
                                        NodeId timeLiteral,
                                        ErrorContainer* errors,
                                        SymbolTable* symbols) {
  NodeId numNode = fC->Child(timeLiteral);
  if (!numNode) return;

  const VObjectType kNumType = fC->Type(numNode);
  if (kNumType != VObjectType::slIntConst &&
      kNumType != VObjectType::slRealConst) {
    return;
  }

  NodeId timeUnitNode = fC->Sibling(numNode);
  if (!timeUnitNode) return;
  if (fC->Type(timeUnitNode) != VObjectType::paTime_unit) return;

  if (!TokenContainsExponent(fC, numNode)) return;

  const PathId fileId = fC->getFileId(numNode);
  const uint32_t line = fC->Line(numNode);
  const uint32_t colStart = fC->Column(numNode);
  const uint32_t colEnd = fC->EndColumn(numNode);
  const std::string_view kUnit = fC->SymName(timeUnitNode);

  std::string originalNum = GetTokenText(fileId, line, colStart, colEnd);
  if (originalNum.empty()) originalNum = fC->SymName(numNode);

  std::string badValue;
  badValue.reserve(originalNum.size() + kUnit.size());
  badValue.append(originalNum).append(kUnit);

  ReportError(fC, numNode, badValue,
              ErrorDefinition::LINT_EXPONENT_FORMAT_TIME_VALUE, errors,
              symbols);
}

void CheckExponentFormatTimeValue(const FileContent* fC, ErrorContainer* errors,
                                  SymbolTable* symbols) {
  if (!fC || !errors || !symbols) return;

  NodeId root = fC->getRootNode();
  if (!root) return;

  for (NodeId timeLiteral :
       fC->sl_collect_all(root, VObjectType::paTime_literal)) {
    CheckTimeLiteralForExponent(fC, timeLiteral, errors, symbols);
  }
}