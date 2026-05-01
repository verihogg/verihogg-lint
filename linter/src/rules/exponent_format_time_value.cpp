#include "rules/exponent_format_time_value.h"

#include <Surelog/Common/FileSystem.h>
#include <Surelog/Common/NodeId.h>
#include <Surelog/Common/PathId.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include "fix/fix_it.h"
#include "main/lint_diagnostics.h"
#include "main/lint_rules.h"
#include "utils/location_utils.h"

namespace SL = SURELOG;

using LineCache = std::vector<std::string>;

namespace {
auto GetLines(SL::PathId fileId) -> const LineCache& {
  static std::unordered_map<uint32_t, LineCache> sFileCache;
  const auto kKey = static_cast<uint32_t>(fileId);
  auto cacheIt = sFileCache.find(kKey);
  if (cacheIt != sFileCache.end()) {
    return cacheIt->second;
  }

  LineCache& lines = sFileCache[kKey];

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

  const std::string& srcLine = lines.at(line - 1);

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

  return std::ranges::any_of(
      kText, [](char chr) -> bool { return chr == 'e' || chr == 'E'; });
}

auto ExpandExponent(std::string_view text) -> std::string {
  const auto ePos = text.find_first_of("eE");
  if (ePos == std::string_view::npos) {
    return {};
  }

  const std::string_view mantissa_sv = text.substr(0, ePos);
  const std::string_view exp_sv = text.substr(ePos + 1);
  if (mantissa_sv.empty() || exp_sv.empty()) {
    return {};
  }

  int exp_val = 0;
  bool exp_neg = false;
  size_t exp_i = 0;
  if (exp_sv.at(exp_i) == '+' || exp_sv.at(exp_i) == '-') {
    exp_neg = (exp_sv.at(exp_i) == '-');
    ++exp_i;
  }
  constexpr int kDecimalBase = 10;
  const size_t exp_digit_start = exp_i;
  for (; exp_i < exp_sv.size(); ++exp_i) {
    if (exp_sv.at(exp_i) < '0' || exp_sv.at(exp_i) > '9') {
      return {};
    }
    exp_val = exp_val * kDecimalBase + (exp_sv.at(exp_i) - '0');
  }
  if (exp_i == exp_digit_start) {
    return {};
  }
  if (exp_neg) {
    exp_val = -exp_val;
  }

  const auto dotPos = mantissa_sv.find('.');
  std::string digits;
  int frac_digits = 0;
  if (dotPos == std::string_view::npos) {
    digits = std::string(mantissa_sv);
  } else {
    digits = std::string(mantissa_sv.substr(0, dotPos)) +
             std::string(mantissa_sv.substr(dotPos + 1));
    frac_digits = static_cast<int>(mantissa_sv.size() - dotPos - 1);
  }
  for (char c : digits) {
    if (c < '0' || c > '9') {
      return {};
    }
  }
  if (digits.empty()) {
    return {};
  }

  const int net = exp_val - frac_digits;
  const int dlen = static_cast<int>(digits.size());

  std::string result;
  if (net >= 0) {
    result = digits + std::string(static_cast<size_t>(net), '0');
  } else {
    const int abs_net = -net;
    if (abs_net >= dlen) {
      result =
          "0." + std::string(static_cast<size_t>(abs_net - dlen), '0') + digits;
    } else {
      const auto split = static_cast<size_t>(dlen - abs_net);
      result = digits.substr(0, split) + "." + digits.substr(split);
    }
  }

  const auto dot_in_result = result.find('.');
  const size_t int_end =
      (dot_in_result == std::string::npos) ? result.size() : dot_in_result;
  const size_t first_nz = result.find_first_not_of('0');
  if (first_nz == std::string::npos || first_nz >= int_end) {
    result = "0" + result.substr(int_end);
  } else if (first_nz > 0) {
    result = result.substr(first_nz);
  }

  return result;
}

}  // namespace

auto CheckExponentFormatTimeValueFixable(const SL::FileContent* fileContent,
                                         SL::ErrorContainer* errors,
                                         SL::SymbolTable* symbols,
                                         [[maybe_unused]] FixSourceManager& sm)
    -> std::vector<LintDiagnostic> {
  std::vector<LintDiagnostic> diags;

  if (fileContent == nullptr || errors == nullptr || symbols == nullptr) {
    return diags;
  }

  SL::NodeId const kRoot = fileContent->getRootNode();
  if (!kRoot) {
    return diags;
  }

  const std::string filepath = GetFixFilepath(fileContent);
  if (filepath.empty()) {
    return diags;
  }

  for (SL::NodeId const kTimeLiteral :
       fileContent->sl_collect_all(kRoot, SL::VObjectType::paTime_literal)) {
    SL::NodeId const kNumNode = fileContent->Child(kTimeLiteral);
    if (!kNumNode) {
      continue;
    }

    const SL::VObjectType kNumType = fileContent->Type(kNumNode);
    if (kNumType != SL::VObjectType::slIntConst &&
        kNumType != SL::VObjectType::slRealConst) {
      continue;
    }

    SL::NodeId const kTimeUnitNode = fileContent->Sibling(kNumNode);
    if (!kTimeUnitNode) {
      continue;
    }
    if (fileContent->Type(kTimeUnitNode) != SL::VObjectType::paTime_unit) {
      continue;
    }

    if (!TokenContainsExponent(fileContent, kNumNode)) {
      continue;
    }

    const SL::PathId kFileId = fileContent->getFileId(kNumNode);
    const auto kLine = static_cast<unsigned>(fileContent->Line(kNumNode));
    const auto kColStart = static_cast<unsigned>(fileContent->Column(kNumNode));
    const auto kColEnd =
        static_cast<unsigned>(fileContent->EndColumn(kNumNode));
    const std::string_view kUnit = fileContent->SymName(kTimeUnitNode);

    std::string originalNum = GetTokenText(kFileId, kLine, kColStart, kColEnd);
    if (originalNum.empty()) {
      originalNum = std::string(fileContent->SymName(kNumNode));
    }

    std::string badValue;
    badValue.reserve(originalNum.size() + kUnit.size());
    badValue.append(originalNum).append(kUnit);

    ReportError(fileContent, kNumNode, badValue,
                verihogg_lint::LINT_EXPONENT_FORMAT_TIME_VALUE, errors,
                symbols);

    const std::string expanded = ExpandExponent(originalNum);
    if (expanded.empty()) {
      continue;
    }

    if (kLine == 0 || kColStart == 0 || kColEnd == 0 || kColEnd <= kColStart) {
      continue;
    }

    const FixRange replace_range{
        .begin =
            FixLocation{.filename = filepath, .line = kLine, .col = kColStart},
        .end =
            FixLocation{.filename = filepath, .line = kLine, .col = kColEnd}};

    LintDiagnostic d;
    d.filepath = filepath;
    d.line = kLine;
    d.col = kColStart;
    d.message.reserve(originalNum.size() + kUnit.size() + 4 + expanded.size() +
                      kUnit.size());
    d.message.append(originalNum)
        .append(kUnit)
        .append(" -> ")
        .append(expanded)
        .append(kUnit);

    try {
      d.fixes.push_back(
          FixIt::Replace(replace_range, expanded,
                         "replace exponent notation with plain decimal"));
    } catch (const std::exception& e) {
      std::cerr << "autofix: cannot create fix at " << filepath << ":" << kLine
                << ":" << kColStart << ": " << e.what() << "\n";
      diags.push_back(std::move(d));
      continue;
    }

    diags.push_back(std::move(d));
  }

  return diags;
}
