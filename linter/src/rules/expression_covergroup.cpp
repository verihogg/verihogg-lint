#include <algorithm>
#include <array>
#include <string_view>
#include <unordered_set>

#include "Surelog/Design/FileContent.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"
#include "Surelog/SourceCompile/VObjectTypes.h"
#include "rules/covergroup_expression.h"
#include "utils/location_utils.h"

using namespace SURELOG;

static constexpr std::array kBinTypes = {
    VObjectType::paBins_Bins,
    VObjectType::paBins_Ignore,
    VObjectType::paBins_Illegal,
};

static std::unordered_set<std::string_view> getAllowedCovergroupArgs(
    const FileContent* fC, NodeId covergroupNode) {
  std::unordered_set<std::string_view> allowed;

  for (NodeId eventId : fC->sl_collect_all(
           covergroupNode, VObjectType::paCoverage_event, false)) {
    for (NodeId tfListId :
         fC->sl_collect_all(eventId, VObjectType::paTf_port_list, false)) {
      for (NodeId tfId :
           fC->sl_collect_all(tfListId, VObjectType::paTf_port_item, false)) {
        for (NodeId nameNode :
             fC->sl_collect_all(tfId, VObjectType::slStringConst, false)) {
          allowed.insert(fC->SymName(nameNode));
        }
      }
    }
  }

  return allowed;
}

static std::unordered_set<std::string_view> getModuleVariables(
    const FileContent* fC) {
  std::unordered_set<std::string_view> moduleVars;

  for (NodeId varDeclId : fC->sl_collect_all(
           fC->getRootNode(), VObjectType::paVariable_declaration)) {
    for (NodeId assignId : fC->sl_collect_all(
             varDeclId, VObjectType::paVariable_decl_assignment, false)) {
      NodeId nameNode = fC->Child(assignId);
      if (nameNode && fC->Type(nameNode) == VObjectType::slStringConst) {
        moduleVars.insert(fC->SymName(nameNode));
      }
    }
  }

  return moduleVars;
}

static void checkIdentifiersRecursive(
    const FileContent* fC, NodeId node,
    const std::unordered_set<std::string_view>& allowedArgs,
    const std::unordered_set<std::string_view>& moduleVars,
    ErrorContainer* errors, SymbolTable* symbols) {
  if (!node) return;

  if (fC->Type(node) == VObjectType::slStringConst) {
    std::string_view varName = fC->SymName(node);

    if (moduleVars.contains(varName)) {
      if (!allowedArgs.contains(varName)) {
        reportError(fC, node, varName,
                    ErrorDefinition::LINT_COVERGROUP_EXPRESSION, errors,
                    symbols);
        return;
      }
    }
  }

  for (NodeId child = fC->Child(node); child; child = fC->Sibling(child)) {
    checkIdentifiersRecursive(fC, child, allowedArgs, moduleVars, errors,
                              symbols);
  }
}

static void checkBinsInCoverpoint(
    const FileContent* fC, NodeId coverpointNode,
    const std::unordered_set<std::string_view>& allowedArgs,
    const std::unordered_set<std::string_view>& moduleVars,
    ErrorContainer* errors, SymbolTable* symbols) {
  if (!coverpointNode) return;

  for (NodeId binsOptId : fC->sl_collect_all(
           coverpointNode, VObjectType::paBins_or_options, false)) {
    bool foundBinType = false;
    std::string_view binName = "unknown";

    for (NodeId n = fC->Child(binsOptId); n; n = fC->Sibling(n)) {
      VObjectType t = fC->Type(n);

      if (std::ranges::any_of(kBinTypes,
                              [t](VObjectType bt) { return bt == t; })) {
        foundBinType = true;
        continue;
      }

      if (foundBinType && t == VObjectType::slStringConst &&
          binName == "<unknown>") {
        binName = fC->SymName(n);
        continue;
      }
    }

    checkIdentifiersRecursive(fC, binsOptId, allowedArgs, moduleVars, errors,
                              symbols);
  }
}

void checkCovergroupExpression(const FileContent* fC, ErrorContainer* errors,
                               SymbolTable* symbols) {
  if (!fC || !errors || !symbols) return;

  NodeId root = fC->getRootNode();
  if (!root) return;

  auto moduleVars = getModuleVariables(fC);

  for (NodeId cgId :
       fC->sl_collect_all(root, VObjectType::paCovergroup_declaration)) {
    auto allowedArgs = getAllowedCovergroupArgs(fC, cgId);
    for (NodeId cpId :
         fC->sl_collect_all(cgId, VObjectType::paCover_point, false)) {
      checkBinsInCoverpoint(fC, cpId, allowedArgs, moduleVars, errors, symbols);
    }
  }
}