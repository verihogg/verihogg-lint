#include "rules/assignment_pattern.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <algorithm>
#include <string_view>
#include <unordered_set>

#include "main/lint_rules.h"
#include "utils/ast_utils.h"
#include "utils/location_utils.h"
#include "utils/name_utils.h"

namespace SL = SURELOG;

namespace {
auto CollectStructTypeNames(const SL::FileContent* fileContent,
                            SL::NodeId moduleRoot)
    -> std::unordered_set<std::string_view> {
  std::unordered_set<std::string_view> structTypeNames;
  auto typeDecls = fileContent->sl_collect_all(
      moduleRoot, SL::VObjectType::paType_declaration);
  for (SL::NodeId const typeDecl : typeDecls) {
    if (typeDecl == SL::InvalidNodeId) {
      continue;
    }
    if (fileContent->sl_collect_all(typeDecl, SL::VObjectType::paStruct_union)
            .empty()) {
      continue;
    }
    for (SL::NodeId child = fileContent->Child(typeDecl);
         child != SL::InvalidNodeId; child = fileContent->Sibling(child)) {
      if (fileContent->Type(child) == SL::VObjectType::slStringConst) {
        structTypeNames.insert(fileContent->SymName(child));
      }
    }
  }
  return structTypeNames;
}

auto IsStructViaVarDecl(
    const SL::FileContent* fileContent, SL::NodeId moduleRoot,
    std::string_view varName,
    const std::unordered_set<std::string_view>& structTypeNames) -> bool {
  auto varDecls = fileContent->sl_collect_all(
      moduleRoot, SL::VObjectType::paVariable_declaration);
  return std::ranges::any_of(varDecls, [&](SL::NodeId varDecl) {
    if (varDecl == SL::InvalidNodeId) {
      return false;
    }
    if (ExtractVariableName(fileContent, varDecl) != varName) {
      return false;
    }
    SL::NodeId const dataType = fileContent->Child(varDecl);
    if (dataType == SL::InvalidNodeId) {
      return false;
    }
    if (!fileContent->sl_collect_all(dataType, SL::VObjectType::paStruct_union)
             .empty()) {
      return true;
    }
    SL::NodeId const dtChild = fileContent->Child(dataType);
    return dtChild != SL::InvalidNodeId &&
           fileContent->Type(dtChild) == SL::VObjectType::slStringConst &&
           structTypeNames.contains(fileContent->SymName(dtChild));
  });
  return false;
}

auto NetDeclMatchesName(const SL::FileContent* fileContent, SL::NodeId netDecl,
                        std::string_view varName) -> bool {
  auto assignNodes = fileContent->sl_collect_all(
      netDecl, SL::VObjectType::paNet_decl_assignment);
  return std::ranges::any_of(assignNodes, [&](SL::NodeId assignNode) {
    SL::NodeId const nameNode = fileContent->Child(assignNode);
    return nameNode != SL::InvalidNodeId &&
           fileContent->Type(nameNode) == SL::VObjectType::slStringConst &&
           fileContent->SymName(nameNode) == varName;
  });
  return false;
}

auto IsStructViaNetDecl(
    const SL::FileContent* fileContent, SL::NodeId moduleRoot,
    std::string_view varName,
    const std::unordered_set<std::string_view>& structTypeNames) -> bool {
  auto netDecls = fileContent->sl_collect_all(
      moduleRoot, SL::VObjectType::paNet_declaration);
  return std::ranges::any_of(netDecls, [&](SL::NodeId netDecl) {
    if (netDecl == SL::InvalidNodeId) {
      return false;
    }
    if (!NetDeclMatchesName(fileContent, netDecl, varName)) {
      return false;
    }
    if (!fileContent->sl_collect_all(netDecl, SL::VObjectType::paStruct_union)
             .empty()) {
      return true;
    }
    SL::NodeId const firstChild = fileContent->Child(netDecl);
    return firstChild != SL::InvalidNodeId &&
           fileContent->Type(firstChild) == SL::VObjectType::slStringConst &&
           structTypeNames.contains(fileContent->SymName(firstChild));
  });
  return false;
}

auto IsStructVariable(const SL::FileContent* fileContent, SL::NodeId moduleRoot,
                      std::string_view varName) -> bool {
  if (varName.empty() || varName == "<unknown>") {
    return false;
  }

  const auto kStructTypeNames = CollectStructTypeNames(fileContent, moduleRoot);

  return IsStructViaVarDecl(fileContent, moduleRoot, varName,
                            kStructTypeNames) ||
         IsStructViaNetDecl(fileContent, moduleRoot, varName, kStructTypeNames);
}

auto IsArrayVariable(const SL::FileContent* fileContent, SL::NodeId moduleRoot,
                     std::string_view varName) -> bool {
  if (varName.empty() || varName == "<unknown>") {
    return false;
  }

  auto vdas = fileContent->sl_collect_all(
      moduleRoot, SL::VObjectType::paVariable_decl_assignment);
  for (SL::NodeId const vda : vdas) {
    if (vda == SL::InvalidNodeId) {
      continue;
    }
    SL::NodeId const nameNode = fileContent->Child(vda);
    if (nameNode == SL::InvalidNodeId ||
        fileContent->Type(nameNode) != SL::VObjectType::slStringConst) {
      continue;
    }
    if (fileContent->SymName(nameNode) != varName) {
      continue;
    }
    if (!fileContent->sl_collect_all(vda, SL::VObjectType::paUnpacked_dimension)
             .empty()) {
      return true;
    }
  }

  auto netDecls = fileContent->sl_collect_all(
      moduleRoot, SL::VObjectType::paNet_declaration);
  for (SL::NodeId const netDecl : netDecls) {
    if (netDecl == SL::InvalidNodeId) {
      continue;
    }
    if (fileContent
            ->sl_collect_all(netDecl, SL::VObjectType::paUnpacked_dimension)
            .empty()) {
      continue;
    }
    auto assignNodes = fileContent->sl_collect_all(
        netDecl, SL::VObjectType::paNet_decl_assignment);
    for (SL::NodeId const assignNode : assignNodes) {
      SL::NodeId const nameNode = fileContent->Child(assignNode);
      if (nameNode != SL::InvalidNodeId &&
          fileContent->Type(nameNode) == SL::VObjectType::slStringConst &&
          fileContent->SymName(nameNode) == varName) {
        return true;
      }
    }
  }

  return false;
}
}  // namespace

void CheckAssignmentPattern(const SL::FileContent* fileContent,
                            SL::ErrorContainer* errors,
                            SL::SymbolTable* symbols) {
  if (fileContent == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }

  SL::NodeId const root = fileContent->getRootNode();
  if (!root) {
    return;
  }

  for (SL::NodeId const concat :
       fileContent->sl_collect_all(root, SL::VObjectType::paConcatenation)) {
    if (!concat) {
      continue;
    }

    SL::NodeId const moduleRoot = FindEnclosingModule(fileContent, concat);
    if (!moduleRoot) {
      continue;
    }

    bool hasLabel = false;
    for (SL::NodeId child = fileContent->Child(concat); child;
         child = fileContent->Sibling(child)) {
      if (fileContent->Type(child) == SL::VObjectType::paArray_member_label) {
        hasLabel = true;
        break;
      }
    }

    std::string_view const varName = FindDirectRhsLhsName(fileContent, concat);
    if (varName == "<unknown>" || varName == "<indexed>") {
      continue;
    }

    if (hasLabel) {
      ReportError(fileContent, concat, varName,
                  verihogg_lint::LINT_ASSIGNMENT_PATTERN, errors, symbols);
      continue;
    }

    if (IsStructVariable(fileContent, moduleRoot, varName) ||
        IsArrayVariable(fileContent, moduleRoot, varName)) {
      ReportError(fileContent, concat, varName,
                  verihogg_lint::LINT_ASSIGNMENT_PATTERN, errors, symbols);
    }
  }
}
