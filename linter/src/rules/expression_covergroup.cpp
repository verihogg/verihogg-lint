#include <cstdint>
#include <set>
#include <string>

#include "Surelog/Design/Design.h"
#include "Surelog/Design/FileContent.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"
#include "Surelog/SourceCompile/VObjectTypes.h"
#include "rules/covergroup_expression.h"
#include "utils/location_utils.h"

using namespace SURELOG;

std::set<std::string> getAllowedCovergroupArgs(const FileContent* fC,
                                               NodeId covergroupNode) {
  std::set<std::string> allowed;

  auto coverageEvents =
      fC->sl_collect_all(covergroupNode, VObjectType::paCoverage_event, false);

  for (NodeId eventId : coverageEvents) {
    auto tfPortLists =
        fC->sl_collect_all(eventId, VObjectType::paTf_port_list, false);

    for (NodeId tfListId : tfPortLists) {
      auto tfItems =
          fC->sl_collect_all(tfListId, VObjectType::paTf_port_item, false);

      for (NodeId tfId : tfItems) {
        auto allChildren =
            fC->sl_collect_all(tfId, VObjectType::slStringConst, false);
        for (NodeId nameNode : allChildren) {
          std::string argName = std::string(fC->SymName(nameNode));
          allowed.insert(argName);
        }
      }
    }
  }

  return allowed;
}

std::set<std::string> getModuleVariables(const FileContent* fC) {
  std::set<std::string> moduleVars;

  auto varDeclNodes = fC->sl_collect_all(fC->getRootNode(),
                                         VObjectType::paVariable_declaration);

  for (NodeId varDeclId : varDeclNodes) {
    auto assignNodes = fC->sl_collect_all(
        varDeclId, VObjectType::paVariable_decl_assignment, false);

    for (NodeId assignId : assignNodes) {
      NodeId nameNode = fC->Child(assignId);
      if (nameNode && fC->Type(nameNode) == VObjectType::slStringConst) {
        std::string varName = std::string(fC->SymName(nameNode));
        moduleVars.insert(varName);
      }
    }
  }

  return moduleVars;
}

void checkIdentifiersRecursive(const FileContent* fC, NodeId node,
                               const std::set<std::string>& allowedArgs,
                               const std::set<std::string>& moduleVars,
                               const std::string& binName,
                               ErrorContainer* errors, SymbolTable* symbols) {
  if (!node) return;

  VObjectType type = fC->Type(node);

  if (type == VObjectType::slStringConst) {
    std::string varName = std::string(fC->SymName(node));

    if (moduleVars.count(varName) > 0) {
      if (allowedArgs.count(varName) == 0) {
        reportError(fC, node, varName,
                    ErrorDefinition::LINT_COVERGROUP_EXPRESSION, errors,
                    symbols);
        return;
      }
    }
  }

  for (NodeId child = fC->Child(node); child; child = fC->Sibling(child)) {
    checkIdentifiersRecursive(fC, child, allowedArgs, moduleVars, binName,
                              errors, symbols);
  }
}

void checkBinsInCoverpoint(const FileContent* fC, NodeId coverpointNode,
                           const std::set<std::string>& allowedArgs,
                           const std::set<std::string>& moduleVars,
                           ErrorContainer* errors, SymbolTable* symbols) {
  if (!coverpointNode) return;

  auto binsOptions =
      fC->sl_collect_all(coverpointNode, VObjectType::paBins_or_options, false);

  for (NodeId binsOptId : binsOptions) {
    std::string binName = "<unknown>";
    bool foundBinType = false;

    for (NodeId n = fC->Child(binsOptId); n; n = fC->Sibling(n)) {
      VObjectType t = fC->Type(n);

      if (t == VObjectType::paBins_Bins || t == VObjectType::paBins_Ignore ||
          t == VObjectType::paBins_Illegal) {
        foundBinType = true;
        continue;
      }

      if (foundBinType && t == VObjectType::slStringConst &&
          binName == "<unknown>") {
        binName = std::string(fC->SymName(n));
        continue;
      }
    }

    checkIdentifiersRecursive(fC, binsOptId, allowedArgs, moduleVars, binName,
                              errors, symbols);
  }
}

void checkCovergroupExpression(const FileContent* fC, ErrorContainer* errors,
                               SymbolTable* symbols) {
  NodeId root = fC->getRootNode();

  std::set<std::string> moduleVars = getModuleVariables(fC);

  auto covergroups =
      fC->sl_collect_all(root, VObjectType::paCovergroup_declaration);

  for (NodeId cgId : covergroups) {
    std::set<std::string> allowedArgs = getAllowedCovergroupArgs(fC, cgId);

    auto coverpoints =
        fC->sl_collect_all(cgId, VObjectType::paCover_point, false);

    for (NodeId cpId : coverpoints) {
      checkBinsInCoverpoint(fC, cpId, allowedArgs, moduleVars, errors, symbols);
    }
  }
}