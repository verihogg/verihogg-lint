#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/ErrorReporting/ErrorDefinition.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <algorithm>
#include <stack>
#include <string_view>
#include <unordered_set>
#include <vector>

#include "rules/target_unpacked_array_concatenation.h"
#include "utils/location_utils.h"

namespace SL = SURELOG;

namespace {
auto CollectUnpackedVarsInModule(const SL::FileContent* fileContent,
                                 SL::NodeId moduleNode)
    -> std::unordered_set<std::string_view> {
  std::unordered_set<std::string_view> unpacked;

  for (SL::NodeId const assignId : fileContent->sl_collect_all(
           moduleNode, SL::VObjectType::paVariable_decl_assignment)) {
    SL::NodeId const nameNode = fileContent->Child(assignId);
    if (!nameNode ||
        fileContent->Type(nameNode) != SL::VObjectType::slStringConst) {
      continue;
    }

    std::string_view const name = fileContent->SymName(nameNode);

    for (SL::NodeId sib = fileContent->Sibling(nameNode); sib;
         sib = fileContent->Sibling(sib)) {
      if (fileContent->Type(sib) != SL::VObjectType::paVariable_dimension) {
        continue;
      }
      SL::NodeId const dimChild = fileContent->Child(sib);
      if (dimChild && fileContent->Type(dimChild) ==
                          SL::VObjectType::paUnpacked_dimension) {
        unpacked.insert(name);
        break;
      }
    }
  }

  return unpacked;
}

auto FindFirstUnpackedVarInSubtree(
    const SL::FileContent* fileContent, SL::NodeId node,
    const std::unordered_set<std::string_view>& unpackedVars)
    -> std::string_view {
  if (!node) {
    return "";
  }

  std::stack<SL::NodeId> worklist;
  worklist.push(node);

  while (!worklist.empty()) {
    SL::NodeId const node = worklist.top();
    worklist.pop();

    if (fileContent->Type(node) == SL::VObjectType::slStringConst) {
      std::string_view const name = fileContent->SymName(node);
      if (unpackedVars.contains(name)) {
        return name;
      }
    }

    std::vector<SL::NodeId> children;
    for (SL::NodeId child = fileContent->Child(node); child;
         child = fileContent->Sibling(child)) {
      children.push_back(child);
    }
    std::ranges::reverse(children);
    for (SL::NodeId const child : children) {
      worklist.push(child);
    }
  }

  return "";
}

void CheckVariableLvalueConcatenations(
    const SL::FileContent* fileContent, SL::NodeId moduleNode,
    const std::unordered_set<std::string_view>& unpackedVars,
    SL::ErrorContainer* errors, SL::SymbolTable* symbols) {
  for (SL::NodeId const lvalueId : fileContent->sl_collect_all(
           moduleNode, SL::VObjectType::paVariable_lvalue)) {
    SL::NodeId const firstChild = fileContent->Child(lvalueId);
    if (!firstChild) {
      continue;
    }

    if (fileContent->Type(firstChild) != SL::VObjectType::paVariable_lvalue) {
      continue;
    }

    if (auto foundVar =
            FindFirstUnpackedVarInSubtree(fileContent, lvalueId, unpackedVars);
        !foundVar.empty()) {
      ReportError(fileContent, lvalueId, foundVar,
                  SL::ErrorDefinition::LINT_TARGET_UNPACKED_ARRAY_CONCATENATION,
                  errors, symbols);
    }
  }
}

void CheckNamedPortConnectionConcatenations(
    const SL::FileContent* fileContent, SL::NodeId moduleNode,
    const std::unordered_set<std::string_view>& unpackedVars,
    SL::ErrorContainer* errors, SL::SymbolTable* symbols) {
  for (SL::NodeId const portConnId : fileContent->sl_collect_all(
           moduleNode, SL::VObjectType::paNamed_port_connection)) {
    for (SL::NodeId const concatId : fileContent->sl_collect_all(
             portConnId, SL::VObjectType::paConcatenation)) {
      if (auto foundVar = FindFirstUnpackedVarInSubtree(fileContent, concatId,
                                                        unpackedVars);
          !foundVar.empty()) {
        ReportError(
            fileContent, concatId, foundVar,
            SL::ErrorDefinition::LINT_TARGET_UNPACKED_ARRAY_CONCATENATION,
            errors, symbols);
        break;
      }
    }
  }
}

void CheckSingleModule(const SL::FileContent* fileContent,
                       SL::NodeId moduleNode, SL::ErrorContainer* errors,
                       SL::SymbolTable* symbols) {
  auto unpackedVars = CollectUnpackedVarsInModule(fileContent, moduleNode);
  if (unpackedVars.empty()) {
    return;
  }

  CheckVariableLvalueConcatenations(fileContent, moduleNode, unpackedVars,
                                    errors, symbols);

  CheckNamedPortConnectionConcatenations(fileContent, moduleNode, unpackedVars,
                                         errors, symbols);
}
}  // namespace

void CheckTargetUnpackedArrayConcatenation(const SL::FileContent* fileContent,
                                           SL::ErrorContainer* errors,
                                           SL::SymbolTable* symbols) {
  if (fileContent == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }

  SL::NodeId const root = fileContent->getRootNode();
  if (!root) {
    return;
  }

  for (SL::NodeId const moduleNode : fileContent->sl_collect_all(
           root, SL::VObjectType::paModule_declaration)) {
    CheckSingleModule(fileContent, moduleNode, errors, symbols);
  }
}