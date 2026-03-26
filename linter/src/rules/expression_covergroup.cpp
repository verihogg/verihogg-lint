#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <algorithm>
#include <array>
#include <stack>
#include <string_view>
#include <unordered_set>

#include "main/lint_rules.h"
#include "rules/covergroup_expression.h"
#include "utils/location_utils.h"

namespace SL = SURELOG;

static constexpr std::array kBinTypes = {
    SL::VObjectType::paBins_Bins,
    SL::VObjectType::paBins_Ignore,
    SL::VObjectType::paBins_Illegal,
};

namespace {
auto GetAllowedCovergroupArgs(const SL::FileContent* fileContent,
                              SL::NodeId covergroupNode)
    -> std::unordered_set<std::string_view> {
  std::unordered_set<std::string_view> allowed;

  for (SL::NodeId const kEventId : fileContent->sl_collect_all(
           covergroupNode, SL::VObjectType::paCoverage_event, false)) {
    for (SL::NodeId const kTfListId : fileContent->sl_collect_all(
             kEventId, SL::VObjectType::paTf_port_list, false)) {
      for (SL::NodeId const kTfId : fileContent->sl_collect_all(
               kTfListId, SL::VObjectType::paTf_port_item, false)) {
        for (SL::NodeId const kNameNode : fileContent->sl_collect_all(
                 kTfId, SL::VObjectType::slStringConst, false)) {
          allowed.insert(fileContent->SymName(kNameNode));
        }
      }
    }
  }

  return allowed;
}

auto GetModuleVariables(const SL::FileContent* fileContent)
    -> std::unordered_set<std::string_view> {
  std::unordered_set<std::string_view> moduleVars;

  for (SL::NodeId const kVarDeclId :
       fileContent->sl_collect_all(fileContent->getRootNode(),
                                   SL::VObjectType::paVariable_declaration)) {
    for (SL::NodeId const kAssignId : fileContent->sl_collect_all(
             kVarDeclId, SL::VObjectType::paVariable_decl_assignment, false)) {
      SL::NodeId const kNameNode = fileContent->Child(kAssignId);
      if (kNameNode &&
          fileContent->Type(kNameNode) == SL::VObjectType::slStringConst) {
        moduleVars.insert(fileContent->SymName(kNameNode));
      }
    }
  }

  return moduleVars;
}

void CheckIdentifiersRecursive(
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
    SL::NodeId const kCurrent = stack.top();
    stack.pop();

    if (fileContent->Type(kCurrent) == SL::VObjectType::slStringConst) {
      std::string_view const kVarName = fileContent->SymName(kCurrent);
      if (moduleVars.contains(kVarName) && !allowedArgs.contains(kVarName)) {
        ReportError(fileContent, kCurrent, kVarName,
                    verihogg_lint::LINT_COVERGROUP_EXPRESSION, errors, symbols);
        continue;
      }
    }

    std::stack<SL::NodeId> children;
    for (SL::NodeId child = fileContent->Child(kCurrent); child;
         child = fileContent->Sibling(child)) {
      children.push(child);
    }
    while (!children.empty()) {
      stack.push(children.top());
      children.pop();
    }
  }
}

void CheckBinsInCoverpoint(
    const SL::FileContent* fileContent, SL::NodeId coverpointNode,
    const std::unordered_set<std::string_view>& allowedArgs,
    const std::unordered_set<std::string_view>& moduleVars,
    SL::ErrorContainer* errors, SL::SymbolTable* symbols) {
  if (coverpointNode == SL::InvalidNodeId) {
    return;
  }

  for (SL::NodeId const kBinsOptId : fileContent->sl_collect_all(
           coverpointNode, SL::VObjectType::paBins_or_options, false)) {
    bool foundBinType = false;
    std::string_view binName = "unknown";

    for (SL::NodeId node = fileContent->Child(kBinsOptId); node;
         node = fileContent->Sibling(node)) {
      SL::VObjectType const kType = fileContent->Type(node);

      if (std::ranges::any_of(kBinTypes, [kType](SL::VObjectType binType) {
            return binType == kType;
          })) {
        foundBinType = true;
        continue;
      }

      if (foundBinType && kType == SL::VObjectType::slStringConst &&
          binName == "<unknown>") {
        binName = fileContent->SymName(node);
        continue;
      }
    }

    CheckIdentifiersRecursive(fileContent, kBinsOptId, allowedArgs, moduleVars,
                              errors, symbols);
  }
}
}  // namespace

void CheckCovergroupExpression(const SL::FileContent* fileContent,
                               SL::ErrorContainer* errors,
                               SL::SymbolTable* symbols) {
  if (fileContent == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }

  SL::NodeId const kRoot = fileContent->getRootNode();
  if (kRoot == SL::InvalidNodeId) {
    return;
  }

  auto moduleVars = GetModuleVariables(fileContent);

  for (SL::NodeId const kCgId : fileContent->sl_collect_all(
           kRoot, SL::VObjectType::paCovergroup_declaration)) {
    auto allowedArgs = GetAllowedCovergroupArgs(fileContent, kCgId);
    for (SL::NodeId const kCpId : fileContent->sl_collect_all(
             kCgId, SL::VObjectType::paCover_point, false)) {
      CheckBinsInCoverpoint(fileContent, kCpId, allowedArgs, moduleVars, errors,
                            symbols);
    }
  }
}