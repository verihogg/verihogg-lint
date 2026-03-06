#include "rules/scalar_assignment_pattern.h"

#include <string>

#include "Surelog/Design/FileContent.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"
#include "Surelog/SourceCompile/VObjectTypes.h"
#include "utils/ast_utils.h"
#include "utils/location_utils.h"
#include "utils/name_utils.h"

using namespace SURELOG;

static bool is1BitScalarKeyword(VObjectType type) {
  return type == VObjectType::paIntVec_TypeBit ||
         type == VObjectType::paIntVec_TypeLogic ||
         type == VObjectType::paIntVec_TypeReg;
}

static bool isScalarVariable(const FileContent* fC, NodeId root,
                             NodeId patternNode, const std::string& varName) {
  if (varName.empty() || varName == "<unknown>" || varName == "<indexed>")
    return false;

  NodeId patternModule = findEnclosingModule(fC, patternNode);

  auto varDecls = fC->sl_collect_all(root, VObjectType::paVariable_declaration);

  for (NodeId vd : varDecls) {
    if (!vd) continue;

    if (findEnclosingModule(fC, vd) != patternModule) continue;

    if (extractVariableName(fC, vd) != varName) continue;

    NodeId dataType = fC->Child(vd);
    if (!dataType) continue;

    NodeId typeKeyword = fC->Child(dataType);
    if (!typeKeyword || !is1BitScalarKeyword(fC->Type(typeKeyword))) continue;

    bool hasPacked = false;
    for (NodeId s = fC->Sibling(typeKeyword); s; s = fC->Sibling(s)) {
      if (fC->Type(s) == VObjectType::paPacked_dimension) {
        hasPacked = true;
        break;
      }
    }
    if (hasPacked) continue;

    auto vdas = fC->sl_collect_all(vd, VObjectType::paVariable_decl_assignment);
    bool hasUnpacked = false;
    for (NodeId vda : vdas) {
      NodeId nameNode = fC->Child(vda);
      if (!nameNode) continue;
      for (NodeId s = fC->Sibling(nameNode); s; s = fC->Sibling(s)) {
        VObjectType st = fC->Type(s);
        if (st == VObjectType::paUnpacked_dimension ||
            st == VObjectType::paVariable_dimension) {
          hasUnpacked = true;
          break;
        }
      }
      if (hasUnpacked) break;
    }
    if (hasUnpacked) continue;

    return true;
  }

  return false;
}

void checkScalarAssignmentPattern(const FileContent* fC, ErrorContainer* errors,
                                  SymbolTable* symbols) {
  if (!fC || !errors || !symbols) return;

  NodeId root = fC->getRootNode();
  if (!root) return;

  auto patterns = fC->sl_collect_all(root, VObjectType::paAssignment_pattern);

  for (NodeId pat : patterns) {
    if (!pat) continue;

    std::string varName = findDirectRhsLhsName(fC, pat);

    if (isScalarVariable(fC, root, pat, varName)) {
      reportError(fC, pat, varName,
                  ErrorDefinition::LINT_SCALAR_ASSIGNMENT_PATTERN, errors,
                  symbols);
    }
  }
}