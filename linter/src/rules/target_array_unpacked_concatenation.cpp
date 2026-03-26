#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <algorithm>
#include <stack>
#include <string_view>
#include <unordered_set>
#include <vector>

#include "main/lint_rules.h"
#include "rules/target_unpacked_array_concatenation.h"
#include "utils/location_utils.h"

namespace SL = SURELOG;

namespace {
auto CollectUnpackedVarsInModule(const SL::FileContent* fileContent,
                                 SL::NodeId moduleNode)
    -> std::unordered_set<std::string_view> {
  std::unordered_set<std::string_view> unpacked;

  for (SL::NodeId const kAssignId : fileContent->sl_collect_all(
           moduleNode, SL::VObjectType::paVariable_decl_assignment)) {
    SL::NodeId const kNameNode = fileContent->Child(kAssignId);
    if (!kNameNode ||
        fileContent->Type(kNameNode) != SL::VObjectType::slStringConst) {
      continue;
    }

    std::string_view const kName = fileContent->SymName(kNameNode);

    for (SL::NodeId sib = fileContent->Sibling(kNameNode); sib;
         sib = fileContent->Sibling(sib)) {
      if (fileContent->Type(sib) != SL::VObjectType::paVariable_dimension) {
        continue;
      }
      SL::NodeId const kDimChild = fileContent->Child(sib);
      if (kDimChild && fileContent->Type(kDimChild) ==
                           SL::VObjectType::paUnpacked_dimension) {
        unpacked.insert(kName);
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
    SL::NodeId const kNode = worklist.top();
    worklist.pop();

    if (fileContent->Type(kNode) == SL::VObjectType::slStringConst) {
      std::string_view const kName = fileContent->SymName(kNode);
      if (unpackedVars.contains(kName)) {
        return kName;
      }
    }

    std::vector<SL::NodeId> children;
    for (SL::NodeId child = fileContent->Child(kNode); child;
         child = fileContent->Sibling(child)) {
      children.push_back(child);
    }
    std::ranges::reverse(children);
    for (SL::NodeId const kChild : children) {
      worklist.push(kChild);
    }
  }

  return "";
}

void CheckVariableLvalueConcatenations(
    const SL::FileContent* fileContent, SL::NodeId moduleNode,
    const std::unordered_set<std::string_view>& unpackedVars,
    SL::ErrorContainer* errors, SL::SymbolTable* symbols) {
  for (SL::NodeId const kLvalueId : fileContent->sl_collect_all(
           moduleNode, SL::VObjectType::paVariable_lvalue)) {
    SL::NodeId const kFirstChild = fileContent->Child(kLvalueId);
    if (!kFirstChild) {
      continue;
    }

    if (fileContent->Type(kFirstChild) != SL::VObjectType::paVariable_lvalue) {
      continue;
    }

    if (auto foundVar =
            FindFirstUnpackedVarInSubtree(fileContent, kLvalueId, unpackedVars);
        !foundVar.empty()) {
      ReportError(fileContent, kLvalueId, foundVar,
                  verihogg_lint::LINT_TARGET_UNPACKED_ARRAY_CONCATENATION,
                  errors, symbols);
    }
  }
}

void CheckNamedPortConnectionConcatenations(
    const SL::FileContent* fileContent, SL::NodeId moduleNode,
    const std::unordered_set<std::string_view>& unpackedVars,
    SL::ErrorContainer* errors, SL::SymbolTable* symbols) {
  for (SL::NodeId const kPortConnId : fileContent->sl_collect_all(
           moduleNode, SL::VObjectType::paNamed_port_connection)) {
    for (SL::NodeId const kConcatId : fileContent->sl_collect_all(
             kPortConnId, SL::VObjectType::paConcatenation)) {
      if (auto foundVar = FindFirstUnpackedVarInSubtree(fileContent, kConcatId,
                                                        unpackedVars);
          !foundVar.empty()) {
        ReportError(fileContent, kConcatId, foundVar,
                    verihogg_lint::LINT_TARGET_UNPACKED_ARRAY_CONCATENATION,
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

  SL::NodeId const kRoot = fileContent->getRootNode();
  if (!kRoot) {
    return;
  }

  for (SL::NodeId const kModuleNode : fileContent->sl_collect_all(
           kRoot, SL::VObjectType::paModule_declaration)) {
    CheckSingleModule(fileContent, kModuleNode, errors, symbols);
  }
}