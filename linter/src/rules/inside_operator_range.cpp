#include "rules/inside_operator_range.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/ErrorReporting/ErrorDefinition.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <string_view>

#include "utils/location_utils.h"

namespace SL = SURELOG;

namespace {
auto GetInsideContextName(const SL::FileContent* fileContent,
                          SL::NodeId insideNode) -> std::string_view {
  SL::NodeId const exprNode = fileContent->Parent(insideNode);
  if (exprNode == SL::InvalidNodeId) {
    return "<unknown>";
  }

  SL::NodeId const leftOperand = fileContent->Child(exprNode);
  if (leftOperand == SL::InvalidNodeId) {
    return "<unknown>";
  }

  auto stringNodes =
      fileContent->sl_collect_all(leftOperand, SL::VObjectType::slStringConst);
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

  SL::VObjectType const sibType = fileContent->Type(siblingNode);

  if (sibType == SL::VObjectType::paOpen_range_list) {
    return true;
  }

  if (sibType == SL::VObjectType::paExpression) {
    SL::NodeId const primaryNode = fileContent->Child(siblingNode);
    if (!primaryNode ||
        fileContent->Type(primaryNode) != SL::VObjectType::paPrimary) {
      return false;
    }

    SL::NodeId const concatNode = fileContent->Child(primaryNode);
    if (concatNode &&
        fileContent->Type(concatNode) == SL::VObjectType::paConcatenation) {
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

  SL::NodeId const root = fileContent->getRootNode();
  if (root == SL::InvalidNodeId) {
    return;
  }

  for (SL::NodeId const insideId :
       fileContent->sl_collect_all(root, SL::VObjectType::paINSIDE)) {
    if (!IsValidInsideRange(fileContent, fileContent->Sibling(insideId))) {
      ReportError(
          fileContent, insideId, GetInsideContextName(fileContent, insideId),
          SL::ErrorDefinition::LINT_INSIDE_OPERATOR_RANGE, errors, symbols);
    }
  }
}
