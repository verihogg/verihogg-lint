#include "rules/scalar_assignment_pattern.h"

#include <algorithm>
#include <array>
#include <string_view>

#include "Surelog/Design/FileContent.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"
#include "Surelog/SourceCompile/VObjectTypes.h"
#include "utils/ast_utils.h"
#include "utils/location_utils.h"
#include "utils/name_utils.h"

using namespace SURELOG;

static bool Is1BitScalarKeyword(VObjectType type) {
  static constexpr std::array kScalarTypes = {
      VObjectType::paIntVec_TypeBit,
      VObjectType::paIntVec_TypeLogic,
      VObjectType::paIntVec_TypeReg,
  };
  return std::ranges::any_of(kScalarTypes,
                             [type](VObjectType t) { return t == type; });
}

static bool IsScalarVariable(const FileContent* fC, NodeId root,
                             NodeId patternNode,
                             const std::string_view& varName) {
  if (varName.empty() || varName == "<unknown>" || varName == "<indexed>")
    return false;

  NodeId patternModule = FindEnclosingModule(fC, patternNode);

  for (NodeId vd :
       fC->sl_collect_all(root, VObjectType::paVariable_declaration)) {
    if (FindEnclosingModule(fC, vd) != patternModule) continue;
    if (ExtractVariableName(fC, vd) != varName) continue;

    NodeId dataType = fC->Child(vd);
    if (!dataType) continue;

    NodeId typeKeyword = fC->Child(dataType);
    if (!typeKeyword || !Is1BitScalarKeyword(fC->Type(typeKeyword))) continue;

    if (HasSiblingOfType(fC, typeKeyword, VObjectType::paPacked_dimension))
      continue;

    bool hasUnpacked = false;
    for (NodeId vda :
         fC->sl_collect_all(vd, VObjectType::paVariable_decl_assignment)) {
      NodeId nameNode = fC->Child(vda);
      if (!nameNode) continue;
      if (HasSiblingOfType(fC, nameNode, VObjectType::paUnpacked_dimension) ||
          HasSiblingOfType(fC, nameNode, VObjectType::paVariable_dimension)) {
        hasUnpacked = true;
        break;
      }
    }
    if (hasUnpacked) continue;

    return true;
  }

  return false;
}

void CheckScalarAssignmentPattern(const FileContent* fC, ErrorContainer* errors,
                                  SymbolTable* symbols) {
  if (!fC || !errors || !symbols) return;

  NodeId root = fC->getRootNode();
  if (!root) return;

  for (NodeId pat :
       fC->sl_collect_all(root, VObjectType::paAssignment_pattern)) {
    std::string_view varName = FindDirectRhsLhsName(fC, pat);
    if (IsScalarVariable(fC, root, pat, varName)) {
      ReportError(fC, pat, varName,
                  ErrorDefinition::LINT_SCALAR_ASSIGNMENT_PATTERN, errors,
                  symbols);
    }
  }
}