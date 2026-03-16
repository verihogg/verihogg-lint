#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <algorithm>
#include <array>
#include <stack>
#include <string_view>
#include <unordered_set>

#include "rules/covergroup_expression.h"
#include "utils/location_utils.h"

namespace SL = SURELOG;

static constexpr std::array kBinTypes = {
    SL::VObjectType::paBins_Bins,
    SL::VObjectType::paBins_Ignore,
    SL::VObjectType::paBins_Illegal,
};

static auto GetAllowedCovergroupArgs(const SL::FileContent* fileContent,
                                     SL::NodeId covergroupNode)
    -> std::unordered_set<std::string_view> {
  std::unordered_set<std::string_view> allowed;

  for (SL::NodeId eventId : fileContent->sl_collect_all(
           covergroupNode, SL::VObjectType::paCoverage_event, false)) {
    for (SL::NodeId tfListId : fileContent->sl_collect_all(
             eventId, SL::VObjectType::paTf_port_list, false)) {
      for (SL::NodeId tfId : fileContent->sl_collect_all(
               tfListId, SL::VObjectType::paTf_port_item, false)) {
        for (SL::NodeId nameNode : fileContent->sl_collect_all(
                 tfId, SL::VObjectType::slStringConst, false)) {
          allowed.insert(fileContent->SymName(nameNode));
        }
      }
    }
  }

  return allowed;
}

static auto GetModuleVariables(const SL::FileContent* fileContent)
    -> std::unordered_set<std::string_view> {
  std::unordered_set<std::string_view> moduleVars;

  for (SL::NodeId varDeclId :
       fileContent->sl_collect_all(fileContent->getRootNode(),
                                   SL::VObjectType::paVariable_declaration)) {
    for (SL::NodeId assignId : fileContent->sl_collect_all(
             varDeclId, SL::VObjectType::paVariable_decl_assignment, false)) {
      SL::NodeId nameNode = fileContent->Child(assignId);
      if (nameNode &&
          fileContent->Type(nameNode) == SL::VObjectType::slStringConst) {
        moduleVars.insert(fileContent->SymName(nameNode));
      }
    }
  }

  return moduleVars;
}

static void CheckIdentifiersRecursive(
    const SL::FileContent* fileContent, SL::NodeId node,
    const std::unordered_set<std::string_view>& allowedArgs,
    const std::unordered_set<std::string_view>& moduleVars,
    SL::ErrorContainer* errors, SL::SymbolTable* symbols) {
  if (node == SL::InvalidNodeId) {
    return;
  }

  std::stack<SL::NodeId> stack;
  stack.push(node);

  while (!stack.empty()) {
    SL::NodeId current = stack.top();
    stack.pop();

    if (fileContent->Type(current) == SL::VObjectType::slStringConst) {
      std::string_view varName = fileContent->SymName(current);
      if (moduleVars.contains(varName) && !allowedArgs.contains(varName)) {
        ReportError(fileContent, current, varName,
                    SL::ErrorDefinition::LINT_COVERGROUP_EXPRESSION, errors,
                    symbols);
        continue;
      }
    }

    std::stack<SL::NodeId> children;
    for (SL::NodeId child = fileContent->Child(current); child;
         child = fileContent->Sibling(child)) {
      children.push(child);
    }
    while (!children.empty()) {
      stack.push(children.top());
      children.pop();
    }
  }
}

static void CheckBinsInCoverpoint(
    const SL::FileContent* fileContent, SL::NodeId coverpointNode,
    const std::unordered_set<std::string_view>& allowedArgs,
    const std::unordered_set<std::string_view>& moduleVars,
    SL::ErrorContainer* errors, SL::SymbolTable* symbols) {
  if (coverpointNode == SL::InvalidNodeId) {
    return;
  }

  for (SL::NodeId binsOptId : fileContent->sl_collect_all(
           coverpointNode, SL::VObjectType::paBins_or_options, false)) {
    bool foundBinType = false;
    std::string_view binName = "unknown";

    for (SL::NodeId node = fileContent->Child(binsOptId); node;
         node = fileContent->Sibling(node)) {
      SL::VObjectType type = fileContent->Type(node);

      if (std::ranges::any_of(kBinTypes, [type](SL::VObjectType binType) {
            return binType == type;
          })) {
        foundBinType = true;
        continue;
      }

      if (foundBinType && type == SL::VObjectType::slStringConst &&
          binName == "<unknown>") {
        binName = fileContent->SymName(node);
        continue;
      }
    }

    CheckIdentifiersRecursive(fileContent, binsOptId, allowedArgs, moduleVars,
                              errors, symbols);
  }
}

void CheckCovergroupExpression(const SL::FileContent* fileContent,
                               SL::ErrorContainer* errors,
                               SL::SymbolTable* symbols) {
  if (fileContent == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }

  SL::NodeId root = fileContent->getRootNode();
  if (root == SL::InvalidNodeId) {
    return;
  }

  auto moduleVars = GetModuleVariables(fileContent);

  for (SL::NodeId cgId : fileContent->sl_collect_all(
           root, SL::VObjectType::paCovergroup_declaration)) {
    auto allowedArgs = GetAllowedCovergroupArgs(fileContent, cgId);
    for (SL::NodeId cpId : fileContent->sl_collect_all(
             cgId, SL::VObjectType::paCover_point, false)) {
      CheckBinsInCoverpoint(fileContent, cpId, allowedArgs, moduleVars, errors,
                            symbols);
    }
  }
}