#include <string_view>
#include <unordered_set>

#include "Surelog/Design/FileContent.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"
#include "Surelog/SourceCompile/VObjectTypes.h"
#include "rules/target_unpacked_array_concatenation.h"
#include "utils/location_utils.h"

using namespace SURELOG;

static std::unordered_set<std::string_view> collectUnpackedVarsInModule(
    const FileContent* fC, NodeId moduleNode) {
  std::unordered_set<std::string_view> unpacked;

  for (NodeId assignId : fC->sl_collect_all(
           moduleNode, VObjectType::paVariable_decl_assignment)) {
    NodeId nameNode = fC->Child(assignId);
    if (!nameNode || fC->Type(nameNode) != VObjectType::slStringConst) continue;

    std::string_view name = fC->SymName(nameNode);

    for (NodeId sib = fC->Sibling(nameNode); sib; sib = fC->Sibling(sib)) {
      if (fC->Type(sib) != VObjectType::paVariable_dimension) continue;
      NodeId dimChild = fC->Child(sib);
      if (dimChild && fC->Type(dimChild) == VObjectType::paUnpacked_dimension) {
        unpacked.insert(name);
        break;
      }
    }
  }

  return unpacked;
}

static std::string_view findFirstUnpackedVarInSubtree(
    const FileContent* fC, NodeId node,
    const std::unordered_set<std::string_view>& unpackedVars) {
  if (!node) return "";

  if (fC->Type(node) == VObjectType::slStringConst) {
    std::string_view name = fC->SymName(node);
    if (unpackedVars.count(name)) return name;
  }

  for (NodeId child = fC->Child(node); child; child = fC->Sibling(child)) {
    std::string_view found =
        findFirstUnpackedVarInSubtree(fC, child, unpackedVars);
    if (!found.empty()) return found;
  }

  return "";
}

static void checkVariableLvalueConcatenations(
    const FileContent* fC, NodeId moduleNode,
    const std::unordered_set<std::string_view>& unpackedVars,
    ErrorContainer* errors, SymbolTable* symbols) {
  for (NodeId lvalueId :
       fC->sl_collect_all(moduleNode, VObjectType::paVariable_lvalue)) {
    NodeId firstChild = fC->Child(lvalueId);
    if (!firstChild) continue;

    if (fC->Type(firstChild) != VObjectType::paVariable_lvalue) continue;

    if (auto foundVar =
            findFirstUnpackedVarInSubtree(fC, lvalueId, unpackedVars);
        !foundVar.empty())
      reportError(fC, lvalueId, foundVar,
                  ErrorDefinition::LINT_TARGET_UNPACKED_ARRAY_CONCATENATION,
                  errors, symbols);
  }
}

static void checkNamedPortConnectionConcatenations(
    const FileContent* fC, NodeId moduleNode,
    const std::unordered_set<std::string_view>& unpackedVars,
    ErrorContainer* errors, SymbolTable* symbols) {
  for (NodeId portConnId :
       fC->sl_collect_all(moduleNode, VObjectType::paNamed_port_connection)) {
    for (NodeId concatId :
         fC->sl_collect_all(portConnId, VObjectType::paConcatenation)) {
      if (auto foundVar =
              findFirstUnpackedVarInSubtree(fC, concatId, unpackedVars);
          !foundVar.empty()) {
        reportError(fC, concatId, foundVar,
                    ErrorDefinition::LINT_TARGET_UNPACKED_ARRAY_CONCATENATION,
                    errors, symbols);
        break;
      }
    }
  }
}

static void checkSingleModule(const FileContent* fC, NodeId moduleNode,
                              ErrorContainer* errors, SymbolTable* symbols) {
  auto unpackedVars = collectUnpackedVarsInModule(fC, moduleNode);
  if (unpackedVars.empty()) return;

  checkVariableLvalueConcatenations(fC, moduleNode, unpackedVars, errors,
                                    symbols);

  checkNamedPortConnectionConcatenations(fC, moduleNode, unpackedVars, errors,
                                         symbols);
}

void checkTargetUnpackedArrayConcatenation(const FileContent* fC,
                                           ErrorContainer* errors,
                                           SymbolTable* symbols) {
  if (!fC || !errors || !symbols) return;

  NodeId root = fC->getRootNode();
  if (!root) return;

  for (NodeId moduleNode :
       fC->sl_collect_all(root, VObjectType::paModule_declaration)) {
    checkSingleModule(fC, moduleNode, errors, symbols);
  }
}