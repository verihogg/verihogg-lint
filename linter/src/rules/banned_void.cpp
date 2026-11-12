#include "rules/banned_void.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Common/PathId.h>
#include <Surelog/Common/SymbolId.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/Error.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/ErrorReporting/Location.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <cstddef>
#include <cstdint>
#include <fstream>
#include <set>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "main/lint_rules.h"
#include "utils/location_utils.h"

namespace SL = SURELOG;

namespace {
constexpr std::string_view kBannedToken = "$void";

using LineCache = std::vector<std::string>;

auto ReadSourceLines(const std::string& filepath) -> LineCache {
  LineCache lines;
  if (filepath.empty()) {
    return lines;
  }

  std::ifstream source(filepath);
  if (!source.is_open()) {
    return lines;
  }

  std::string line;
  while (std::getline(source, line)) {
    lines.push_back(std::move(line));
  }
  return lines;
}

auto GetSourceLine(const LineCache& lines, uint32_t line)
    -> const std::string* {
  if (line == 0 || line > lines.size()) {
    return nullptr;
  }
  return &lines.at(line - 1);
}

auto IsIdentifierChar(char chr) -> bool {
  return ((chr >= 'a') && (chr <= 'z')) || ((chr >= 'A') && (chr <= 'Z')) ||
         ((chr >= '0') && (chr <= '9')) || (chr == '_') || (chr == '$');
}

auto StartsWithBannedToken(const std::string& text, uint16_t column) -> bool {
  if (column == 0) {
    return false;
  }

  const auto kStart = static_cast<size_t>(column) - 1;
  if ((kStart + kBannedToken.size()) > text.size()) {
    return false;
  }

  if (std::string_view{text}.substr(kStart, kBannedToken.size()) !=
      kBannedToken) {
    return false;
  }

  const size_t kNext = kStart + kBannedToken.size();
  return (kNext >= text.size()) || !IsIdentifierChar(text.at(kNext));
}

}  // namespace

void CheckBannedVoid(const SL::FileContent* fileContent,
                     SL::ErrorContainer* errors, SL::SymbolTable* symbols) {
  if ((fileContent == nullptr) || (errors == nullptr) || (symbols == nullptr)) {
    return;
  }

  SL::NodeId const kRoot = fileContent->getRootNode();
  if (!kRoot) {
    return;
  }

  const LineCache kLines = ReadSourceLines(GetFixFilepath(fileContent));
  if (kLines.empty()) {
    return;
  }

  const SL::SymbolId kSymbol = symbols->registerSymbol(kBannedToken);
  std::set<std::pair<uint32_t, uint32_t>> reported;

  auto reportIfBanned = [&](SL::NodeId node) {
    const uint32_t kLine = fileContent->Line(node);
    const uint16_t kColumn = fileContent->Column(node);

    const std::string* text = GetSourceLine(kLines, kLine);
    if ((text == nullptr) || !StartsWithBannedToken(*text, kColumn)) {
      return;
    }
    if (!reported.emplace(kLine, static_cast<uint32_t>(kColumn)).second) {
      return;
    }

    const SL::Location kLocation(fileContent->getFileId(node), kLine, kColumn,
                                 kSymbol);
    SL::Error lintError(verihogg_lint::LintId(verihogg_lint::LINT_BANNED_VOID),
                        kLocation);
    errors->addError(lintError, false);
  };

  for (SL::NodeId const kNode :
       fileContent->sl_collect_all(kRoot, SL::VObjectType::paStatement_item)) {
    if (!fileContent->Child(kNode)) {
      reportIfBanned(kNode);
    }
  }

  for (SL::NodeId const kNode :
       fileContent->sl_collect_all(kRoot, SL::VObjectType::paDollar_keyword)) {
    SL::NodeId const kName = fileContent->Sibling(kNode);
    if (!kName ||
        (fileContent->Type(kName) != SL::VObjectType::slStringConst)) {
      reportIfBanned(kNode);
    }
  }
}