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
  for (SL::NodeId const kTypeDecl : typeDecls) {
    if (kTypeDecl == SL::InvalidNodeId) {
      continue;
    }
    if (fileContent->sl_collect_all(kTypeDecl, SL::VObjectType::paStruct_union)
            .empty()) {
      continue;
    }
    for (SL::NodeId child = fileContent->Child(kTypeDecl);
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
    SL::NodeId const kDataType = fileContent->Child(varDecl);
    if (kDataType == SL::InvalidNodeId) {
      return false;
    }
    if (!fileContent->sl_collect_all(kDataType, SL::VObjectType::paStruct_union)
             .empty()) {
      return true;
    }
    SL::NodeId const kDtChild = fileContent->Child(kDataType);
    return kDtChild != SL::InvalidNodeId &&
           fileContent->Type(kDtChild) == SL::VObjectType::slStringConst &&
           structTypeNames.contains(fileContent->SymName(kDtChild));
  });
  return false;
}

auto NetDeclMatchesName(const SL::FileContent* fileContent, SL::NodeId netDecl,
                        std::string_view varName) -> bool {
  auto assignNodes = fileContent->sl_collect_all(
      netDecl, SL::VObjectType::paNet_decl_assignment);
  return std::ranges::any_of(assignNodes, [&](SL::NodeId assignNode) {
    SL::NodeId const kNameNode = fileContent->Child(assignNode);
    return kNameNode != SL::InvalidNodeId &&
           fileContent->Type(kNameNode) == SL::VObjectType::slStringConst &&
           fileContent->SymName(kNameNode) == varName;
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
    SL::NodeId const kFirstChild = fileContent->Child(netDecl);
    return kFirstChild != SL::InvalidNodeId &&
           fileContent->Type(kFirstChild) == SL::VObjectType::slStringConst &&
           structTypeNames.contains(fileContent->SymName(kFirstChild));
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
  for (SL::NodeId const kVda : vdas) {
    if (kVda == SL::InvalidNodeId) {
      continue;
    }
    SL::NodeId const kNameNode = fileContent->Child(kVda);
    if (kNameNode == SL::InvalidNodeId ||
        fileContent->Type(kNameNode) != SL::VObjectType::slStringConst) {
      continue;
    }
    if (fileContent->SymName(kNameNode) != varName) {
      continue;
    }
    if (!fileContent
             ->sl_collect_all(kVda, SL::VObjectType::paUnpacked_dimension)
             .empty()) {
      return true;
    }
  }

  auto netDecls = fileContent->sl_collect_all(
      moduleRoot, SL::VObjectType::paNet_declaration);
  for (SL::NodeId const kNetDecl : netDecls) {
    if (kNetDecl == SL::InvalidNodeId) {
      continue;
    }
    if (fileContent
            ->sl_collect_all(kNetDecl, SL::VObjectType::paUnpacked_dimension)
            .empty()) {
      continue;
    }
    auto assignNodes = fileContent->sl_collect_all(
        kNetDecl, SL::VObjectType::paNet_decl_assignment);
    for (SL::NodeId const kAssignNode : assignNodes) {
      SL::NodeId const kNameNode = fileContent->Child(kAssignNode);
      if (kNameNode != SL::InvalidNodeId &&
          fileContent->Type(kNameNode) == SL::VObjectType::slStringConst &&
          fileContent->SymName(kNameNode) == varName) {
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

  SL::NodeId const kRoot = fileContent->getRootNode();
  if (!kRoot) {
    return;
  }

  for (SL::NodeId const kConcat :
       fileContent->sl_collect_all(kRoot, SL::VObjectType::paConcatenation)) {
    if (!kConcat) {
      continue;
    }

    SL::NodeId const kModuleRoot = FindEnclosingModule(fileContent, kConcat);
    if (!kModuleRoot) {
      continue;
    }

    bool hasLabel = false;
    for (SL::NodeId child = fileContent->Child(kConcat); child;
         child = fileContent->Sibling(child)) {
      if (fileContent->Type(child) == SL::VObjectType::paArray_member_label) {
        hasLabel = true;
        break;
      }
    }

    std::string_view const kVarName =
        FindDirectRhsLhsName(fileContent, kConcat);
    if (kVarName == "<unknown>" || kVarName == "<indexed>") {
      continue;
    }

    if (hasLabel) {
      ReportError(fileContent, kConcat, kVarName,
                  verihogg_lint::LINT_ASSIGNMENT_PATTERN, errors, symbols);
      continue;
    }

    if (IsStructVariable(fileContent, kModuleRoot, kVarName) ||
        IsArrayVariable(fileContent, kModuleRoot, kVarName)) {
      ReportError(fileContent, kConcat, kVarName,
                  verihogg_lint::LINT_ASSIGNMENT_PATTERN, errors, symbols);
    }
  }
}
