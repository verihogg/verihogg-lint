#include "rules/assignment_pattern.h"

#include <string>
#include <unordered_set>

#include "Surelog/Design/FileContent.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"
#include "Surelog/SourceCompile/VObjectTypes.h"
#include "utils/ast_utils.h"
#include "utils/location_utils.h"
#include "utils/name_utils.h"

using namespace SURELOG;

static bool isStructVariable(const FileContent* fC, NodeId moduleRoot,
                             const std::string& varName) {
  if (varName.empty() || varName == "<unknown>") return false;

  std::unordered_set<std::string> structTypeNames;
  auto typeDecls =
      fC->sl_collect_all(moduleRoot, VObjectType::paType_declaration);
  for (NodeId td : typeDecls) {
    if (!td) continue;
    if (fC->sl_collect_all(td, VObjectType::paStruct_union).empty()) continue;
    for (NodeId ch = fC->Child(td); ch; ch = fC->Sibling(ch)) {
      if (fC->Type(ch) == VObjectType::slStringConst)
        structTypeNames.insert(std::string(fC->SymName(ch)));
    }
  }

  auto varDecls =
      fC->sl_collect_all(moduleRoot, VObjectType::paVariable_declaration);
  for (NodeId vd : varDecls) {
    if (!vd) continue;
    if (extractVariableName(fC, vd) != varName) continue;

    NodeId dataType = fC->Child(vd);
    if (!dataType) continue;

    if (!fC->sl_collect_all(dataType, VObjectType::paStruct_union).empty())
      return true;

    NodeId dtChild = fC->Child(dataType);
    if (dtChild && fC->Type(dtChild) == VObjectType::slStringConst &&
        structTypeNames.count(std::string(fC->SymName(dtChild))))
      return true;
  }

  auto netDecls =
      fC->sl_collect_all(moduleRoot, VObjectType::paNet_declaration);
  for (NodeId nd : netDecls) {
    if (!nd) continue;

    auto assignNodes =
        fC->sl_collect_all(nd, VObjectType::paNet_decl_assignment);
    bool nameMatch = false;
    for (NodeId an : assignNodes) {
      NodeId nameNode = fC->Child(an);
      if (nameNode && fC->Type(nameNode) == VObjectType::slStringConst &&
          std::string(fC->SymName(nameNode)) == varName) {
        nameMatch = true;
        break;
      }
    }
    if (!nameMatch) continue;

    if (!fC->sl_collect_all(nd, VObjectType::paStruct_union).empty())
      return true;

    NodeId firstCh = fC->Child(nd);
    if (firstCh && fC->Type(firstCh) == VObjectType::slStringConst &&
        structTypeNames.count(std::string(fC->SymName(firstCh))))
      return true;
  }

  return false;
}

static bool isArrayVariable(const FileContent* fC, NodeId moduleRoot,
                            const std::string& varName) {
  if (varName.empty() || varName == "<unknown>") return false;

  auto vdas =
      fC->sl_collect_all(moduleRoot, VObjectType::paVariable_decl_assignment);
  for (NodeId vda : vdas) {
    if (!vda) continue;
    NodeId nameNode = fC->Child(vda);
    if (!nameNode || fC->Type(nameNode) != VObjectType::slStringConst) continue;
    if (std::string(fC->SymName(nameNode)) != varName) continue;
    if (!fC->sl_collect_all(vda, VObjectType::paUnpacked_dimension).empty())
      return true;
  }

  auto netDecls =
      fC->sl_collect_all(moduleRoot, VObjectType::paNet_declaration);
  for (NodeId nd : netDecls) {
    if (!nd) continue;
    if (fC->sl_collect_all(nd, VObjectType::paUnpacked_dimension).empty())
      continue;
    auto assignNodes =
        fC->sl_collect_all(nd, VObjectType::paNet_decl_assignment);
    for (NodeId an : assignNodes) {
      NodeId nameNode = fC->Child(an);
      if (nameNode && fC->Type(nameNode) == VObjectType::slStringConst &&
          std::string(fC->SymName(nameNode)) == varName)
        return true;
    }
  }

  return false;
}

void checkAssignmentPattern(const FileContent* fC, ErrorContainer* errors,
                            SymbolTable* symbols) {
  if (!fC || !errors || !symbols) return;

  NodeId root = fC->getRootNode();
  if (!root) return;

  for (NodeId concat : fC->sl_collect_all(root, VObjectType::paConcatenation)) {
    if (!concat) continue;

    NodeId moduleRoot = findEnclosingModule(fC, concat);
    if (!moduleRoot) continue;

    bool hasLabel = false;
    for (NodeId child = fC->Child(concat); child; child = fC->Sibling(child)) {
      if (fC->Type(child) == VObjectType::paArray_member_label) {
        hasLabel = true;
        break;
      }
    }

    std::string varName = findDirectRhsLhsName(fC, concat);
    if (varName == "<unknown>" || varName == "<indexed>") continue;

    if (hasLabel) {
      reportError(fC, concat, varName, ErrorDefinition::LINT_ASSIGNMENT_PATTERN,
                  errors, symbols);
      continue;
    }

    if (isStructVariable(fC, moduleRoot, varName) ||
        isArrayVariable(fC, moduleRoot, varName)) {
      reportError(fC, concat, varName, ErrorDefinition::LINT_ASSIGNMENT_PATTERN,
                  errors, symbols);
    }
  }
}
