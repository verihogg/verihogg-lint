#include "rules/assignment_pattern.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <algorithm>
#include <iterator>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

#include "main/lint_rules.h"
#include "utils/ast_utils.h"
#include "utils/location_utils.h"
#include "utils/name_utils.h"

namespace SL = SURELOG;

namespace {
struct ModuleVarTraits {
  std::unordered_set<std::string_view> structVars;
  std::unordered_set<std::string_view> arrayVars;
};

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

auto CollectStructVarsFromVarDecls(
    const SL::FileContent* fileContent, SL::NodeId moduleRoot,
    const std::unordered_set<std::string_view>& structTypeNames,
    std::unordered_set<std::string_view>& structVars) -> void {
  auto varDecls = fileContent->sl_collect_all(
      moduleRoot, SL::VObjectType::paVariable_declaration);
  for (SL::NodeId const varDecl : varDecls) {
    if (varDecl == SL::InvalidNodeId) {
      continue;
    }
    std::string_view const varName = ExtractVariableName(fileContent, varDecl);
    if (varName == "<unknown>") {
      continue;
    }

    SL::NodeId const kDataType = fileContent->Child(varDecl);
    if (kDataType == SL::InvalidNodeId) {
      continue;
    }
    if (!fileContent->sl_collect_all(kDataType, SL::VObjectType::paStruct_union)
             .empty()) {
      structVars.insert(varName);
      continue;
    }
    SL::NodeId const kDtChild = fileContent->Child(kDataType);
    if (kDtChild != SL::InvalidNodeId &&
        fileContent->Type(kDtChild) == SL::VObjectType::slStringConst &&
        structTypeNames.contains(fileContent->SymName(kDtChild))) {
      structVars.insert(varName);
    }
  }
}

auto CollectStructVarsFromNetDecls(
    const SL::FileContent* fileContent, SL::NodeId moduleRoot,
    const std::unordered_set<std::string_view>& structTypeNames,
    std::unordered_set<std::string_view>& structVars) -> void {
  auto netDecls = fileContent->sl_collect_all(
      moduleRoot, SL::VObjectType::paNet_declaration);
  for (SL::NodeId const netDecl : netDecls) {
    if (netDecl == SL::InvalidNodeId) {
      continue;
    }

    if (!fileContent->sl_collect_all(netDecl, SL::VObjectType::paStruct_union)
             .empty()) {
      auto assignNodes = fileContent->sl_collect_all(
          netDecl, SL::VObjectType::paNet_decl_assignment);
      for (SL::NodeId const assignNode : assignNodes) {
        SL::NodeId const nameNode = fileContent->Child(assignNode);
        if (nameNode != SL::InvalidNodeId &&
            fileContent->Type(nameNode) == SL::VObjectType::slStringConst) {
          structVars.insert(fileContent->SymName(nameNode));
        }
      }
      continue;
    }

    SL::NodeId const kFirstChild = fileContent->Child(netDecl);
    if (kFirstChild == SL::InvalidNodeId ||
        fileContent->Type(kFirstChild) != SL::VObjectType::slStringConst ||
        !structTypeNames.contains(fileContent->SymName(kFirstChild))) {
      continue;
    }

    auto assignNodes = fileContent->sl_collect_all(
        netDecl, SL::VObjectType::paNet_decl_assignment);
    for (SL::NodeId const assignNode : assignNodes) {
      SL::NodeId const nameNode = fileContent->Child(assignNode);
      if (nameNode != SL::InvalidNodeId &&
          fileContent->Type(nameNode) == SL::VObjectType::slStringConst) {
        structVars.insert(fileContent->SymName(nameNode));
      }
    }
  }
}

auto CollectArrayVars(const SL::FileContent* fileContent, SL::NodeId moduleRoot)
    -> std::unordered_set<std::string_view> {
  std::unordered_set<std::string_view> result;
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
    if (!fileContent
             ->sl_collect_all(kVda, SL::VObjectType::paUnpacked_dimension)
             .empty()) {
      result.insert(fileContent->SymName(kNameNode));
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
          fileContent->Type(kNameNode) == SL::VObjectType::slStringConst) {
        result.insert(fileContent->SymName(kNameNode));
      }
    }
  }

  return result;
}

auto BuildModuleVarTraits(const SL::FileContent* fileContent,
                          SL::NodeId moduleRoot) -> ModuleVarTraits {
  ModuleVarTraits traits;
  const auto structTypes = CollectStructTypeNames(fileContent, moduleRoot);
  CollectStructVarsFromVarDecls(fileContent, moduleRoot, structTypes,
                                traits.structVars);
  CollectStructVarsFromNetDecls(fileContent, moduleRoot, structTypes,
                                traits.structVars);
  traits.arrayVars = CollectArrayVars(fileContent, moduleRoot);
  return traits;
}

auto IsKnownVariableName(std::string_view varName) -> bool {
  return !varName.empty() && varName != "<unknown>" && varName != "<indexed>";
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

  std::vector<std::pair<SL::NodeId, ModuleVarTraits>> moduleTraitsCache;

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
    if (!IsKnownVariableName(kVarName)) {
      continue;
    }

    auto cacheIt = std::ranges::find_if(
        moduleTraitsCache,
        [kModuleRoot](const auto& item) { return item.first == kModuleRoot; });
    if (cacheIt == moduleTraitsCache.end()) {
      moduleTraitsCache.emplace_back(
          kModuleRoot, BuildModuleVarTraits(fileContent, kModuleRoot));
      cacheIt = std::prev(moduleTraitsCache.end());
    }

    const ModuleVarTraits& traits = cacheIt->second;

    if (hasLabel) {
      ReportError(fileContent, kConcat, kVarName,
                  verihogg_lint::LINT_ASSIGNMENT_PATTERN, errors, symbols);
      continue;
    }

    if (traits.structVars.contains(kVarName) ||
        traits.arrayVars.contains(kVarName)) {
      ReportError(fileContent, kConcat, kVarName,
                  verihogg_lint::LINT_ASSIGNMENT_PATTERN, errors, symbols);
    }
  }
}
