#include "rules/inside_operator_range.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <string_view>

#include "main/lint_rules.h"
#include "utils/location_utils.h"

namespace SL = SURELOG;

namespace {
auto GetInsideContextName(const SL::FileContent* fileContent,
                          SL::NodeId insideNode) -> std::string_view {
  SL::NodeId const kExprNode = fileContent->Parent(insideNode);
  if (kExprNode == SL::InvalidNodeId) {
    return "<unknown>";
  }

  SL::NodeId const kLeftOperand = fileContent->Child(kExprNode);
  if (kLeftOperand == SL::InvalidNodeId) {
    return "<unknown>";
  }

  auto stringNodes =
      fileContent->sl_collect_all(kLeftOperand, SL::VObjectType::slStringConst);
  if (!stringNodes.empty()) {
    return fileContent->SymName(stringNodes.front());
  }

  return "<unknown>";
}

auto IsValidInsideRange(const SL::FileContent* fileContent,
                        SL::NodeId siblingNode) -> bool {
  if (siblingNode == SL::InvalidNodeId) {
    return false;
  }

  SL::VObjectType const kSibType = fileContent->Type(siblingNode);

  if (kSibType == SL::VObjectType::paOpen_range_list) {
    return true;
  }

  if (kSibType == SL::VObjectType::paExpression) {
    SL::NodeId const kPrimaryNode = fileContent->Child(siblingNode);
    if (!kPrimaryNode ||
        fileContent->Type(kPrimaryNode) != SL::VObjectType::paPrimary) {
      return false;
    }

    SL::NodeId const kConcatNode = fileContent->Child(kPrimaryNode);
    if (kConcatNode &&
        fileContent->Type(kConcatNode) == SL::VObjectType::paConcatenation) {
      return true;
    }
  }

  return false;
}
}  // namespace

void CheckInsideOperatorRange(const SL::FileContent* fileContent,
                              SL::ErrorContainer* errors,
                              SL::SymbolTable* symbols) {
  if (fileContent == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }

  SL::NodeId const kRoot = fileContent->getRootNode();
  if (kRoot == SL::InvalidNodeId) {
    return;
  }

  for (SL::NodeId const kInsideId :
       fileContent->sl_collect_all(kRoot, SL::VObjectType::paINSIDE)) {
    if (!IsValidInsideRange(fileContent, fileContent->Sibling(kInsideId))) {
      ReportError(fileContent, kInsideId,
                  GetInsideContextName(fileContent, kInsideId),
                  verihogg_lint::LINT_INSIDE_OPERATOR_RANGE, errors, symbols);
    }
  }
}
