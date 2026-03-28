#include "rules/scalar_assignment_pattern.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <algorithm>
#include <array>
#include <string_view>

#include "main/lint_rules.h"
#include "utils/ast_utils.h"
#include "utils/location_utils.h"
#include "utils/name_utils.h"

namespace SL = SURELOG;

namespace {
auto Is1BitScalarKeyword(SL::VObjectType type) -> bool {
  static constexpr std::array kScalarTypes = {
      SL::VObjectType::paIntVec_TypeBit,
      SL::VObjectType::paIntVec_TypeLogic,
      SL::VObjectType::paIntVec_TypeReg,
  };
  return std::ranges::any_of(kScalarTypes,
                             [type](SL::VObjectType vObjectType) -> bool {
                               return vObjectType == type;
                             });
}

auto IsScalarVariable(const SL::FileContent* fileContent, SL::NodeId root,
                      const std::string_view& varName, SL::NodeId patternNode)
    -> bool {
  if (varName.empty() || varName == "<unknown>" || varName == "<indexed>") {
    return false;
  }

  SL::NodeId const kPatternModule =
      FindEnclosingModule(fileContent, patternNode);

  for (SL::NodeId const kVarDecl : fileContent->sl_collect_all(
           root, SL::VObjectType::paVariable_declaration)) {
    if (FindEnclosingModule(fileContent, kVarDecl) != kPatternModule) {
      continue;
    }
    if (ExtractVariableName(fileContent, kVarDecl) != varName) {
      continue;
    }

    SL::NodeId const kDataType = fileContent->Child(kVarDecl);
    if (kDataType == SL::InvalidNodeId) {
      continue;
    }

    SL::NodeId const kTypeKeyword = fileContent->Child(kDataType);
    if (kTypeKeyword == SL::InvalidNodeId ||
        !Is1BitScalarKeyword(fileContent->Type(kTypeKeyword))) {
      continue;
    }

    if (HasSiblingOfType(fileContent, kTypeKeyword,
                         SL::VObjectType::paPacked_dimension)) {
      continue;
    }

    bool hasUnpacked = false;
    for (SL::NodeId const kVda : fileContent->sl_collect_all(
             kVarDecl, SL::VObjectType::paVariable_decl_assignment)) {
      SL::NodeId const kNameNode = fileContent->Child(kVda);
      if (kNameNode == SL::InvalidNodeId) {
        continue;
      }
      if (HasSiblingOfType(fileContent, kNameNode,
                           SL::VObjectType::paUnpacked_dimension) ||
          HasSiblingOfType(fileContent, kNameNode,
                           SL::VObjectType::paVariable_dimension)) {
        hasUnpacked = true;
        break;
      }
    }
    if (hasUnpacked) {
      continue;
    }

    return true;
  }

  return false;
}
}  // namespace

void CheckScalarAssignmentPattern(const SL::FileContent* fileContent,
                                  SL::ErrorContainer* errors,
                                  SL::SymbolTable* symbols) {
  if (fileContent == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }
  SL::NodeId const kRoot = fileContent->getRootNode();
  if (!kRoot) {
    return;
  }

  for (SL::NodeId const kPat : fileContent->sl_collect_all(
           kRoot, SL::VObjectType::paAssignment_pattern)) {
    std::string_view const kVarName = FindDirectRhsLhsName(fileContent, kPat);
    if (IsScalarVariable(fileContent, kRoot, kVarName, kPat)) {
      ReportError(fileContent, kPat, kVarName,
                  verihogg_lint::LINT_SCALAR_ASSIGNMENT_PATTERN, errors,
                  symbols);
    }
  }
}