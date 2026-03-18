#include "rules/scalar_assignment_pattern.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/ErrorReporting/ErrorDefinition.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <algorithm>
#include <array>
#include <string_view>

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
  return std::ranges::any_of(kScalarTypes, [type](SL::VObjectType vObjectType) {
    return vObjectType == type;
  });
}

auto IsScalarVariable(const SL::FileContent* fileContent, SL::NodeId root,
                      const std::string_view& varName, SL::NodeId patternNode)
    -> bool {
  if (varName.empty() || varName == "<unknown>" || varName == "<indexed>") {
    return false;
  }

  SL::NodeId const patternModule =
      FindEnclosingModule(fileContent, patternNode);

  for (SL::NodeId const varDecl : fileContent->sl_collect_all(
           root, SL::VObjectType::paVariable_declaration)) {
    if (FindEnclosingModule(fileContent, varDecl) != patternModule) {
      continue;
    }
    if (ExtractVariableName(fileContent, varDecl) != varName) {
      continue;
    }

    SL::NodeId const dataType = fileContent->Child(varDecl);
    if (dataType == SL::InvalidNodeId) {
      continue;
    }

    SL::NodeId const typeKeyword = fileContent->Child(dataType);
    if (typeKeyword == SL::InvalidNodeId ||
        !Is1BitScalarKeyword(fileContent->Type(typeKeyword))) {
      continue;
    }

    if (HasSiblingOfType(fileContent, typeKeyword,
                         SL::VObjectType::paPacked_dimension)) {
      continue;
    }

    bool hasUnpacked = false;
    for (SL::NodeId const vda : fileContent->sl_collect_all(
             varDecl, SL::VObjectType::paVariable_decl_assignment)) {
      SL::NodeId const nameNode = fileContent->Child(vda);
      if (nameNode == SL::InvalidNodeId) {
        continue;
      }
      if (HasSiblingOfType(fileContent, nameNode,
                           SL::VObjectType::paUnpacked_dimension) ||
          HasSiblingOfType(fileContent, nameNode,
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
  SL::NodeId const root = fileContent->getRootNode();
  if (!root) {
    return;
  }

  for (SL::NodeId const pat : fileContent->sl_collect_all(
           root, SL::VObjectType::paAssignment_pattern)) {
    std::string_view const varName = FindDirectRhsLhsName(fileContent, pat);
    if (IsScalarVariable(fileContent, root, varName, pat)) {
      ReportError(fileContent, pat, varName,
                  SL::ErrorDefinition::LINT_SCALAR_ASSIGNMENT_PATTERN, errors,
                  symbols);
    }
  }
}