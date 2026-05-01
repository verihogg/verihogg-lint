#include "rules/wildcard_operator_common.h"

#include <Surelog/Common/FileSystem.h>
#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/FileContent.h>

#include <string>
#include <string_view>
#include <vector>

#include "utils/location_utils.h"
#include "utils/name_utils.h"

namespace SL = SURELOG;

namespace {

auto ReadWildcardOperatorLexeme(const SL::FileContent* fileContent,
                                SL::NodeId opNode) -> std::string {
  if (fileContent == nullptr || !opNode) {
    return {};
  }

  const unsigned line = fileContent->Line(opNode);
  const unsigned col = fileContent->Column(opNode);
  if (line == 0 || col == 0) {
    return {};
  }

  SURELOG::FileSystem* const fileSystem = SURELOG::FileSystem::getInstance();

  std::vector<std::string> lines;
  if (!fileSystem->readLines(fileContent->getFileId(), lines)) {
    return {};
  }

  if (line > lines.size()) {
    return {};
  }

  const std::string& srcLine = lines.at(line - 1);
  const auto begin = static_cast<size_t>(col - 1);
  if (begin + 3 > srcLine.size()) {
    return {};
  }

  return srcLine.substr(begin, 3);
}

}  // namespace

auto GetWildcardLhsName(const SL::FileContent* fileContent, SL::NodeId opNode)
    -> std::string_view {
  std::string_view symName = "<unknown>";

  if (fileContent == nullptr || !opNode) {
    return symName;
  }

  const SL::NodeId exprNode = fileContent->Parent(opNode);
  if (!exprNode) {
    return symName;
  }

  const SL::NodeId leftOperand = fileContent->Child(exprNode);
  if (!leftOperand) {
    return symName;
  }

  return ExtractName(fileContent, leftOperand, "<unknown>");
}

auto DetectWildcardOperatorKind(const SL::FileContent* fileContent,
                                SL::NodeId opNode) -> WildcardOperatorKind {
  const std::string op = ReadWildcardOperatorLexeme(fileContent, opNode);
  if (op.size() != 3) {
    return WildcardOperatorKind::kUnknown;
  }

  if (op.at(1) != '?' || op.at(2) != '=') {
    return WildcardOperatorKind::kUnknown;
  }

  if (op.at(0) == '=') {
    return WildcardOperatorKind::kEquality;
  }

  if (op.at(0) == '!') {
    return WildcardOperatorKind::kInequality;
  }

  return WildcardOperatorKind::kUnknown;
}
