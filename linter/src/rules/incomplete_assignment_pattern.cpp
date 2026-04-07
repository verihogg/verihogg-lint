#include "rules/incomplete_assignment_pattern.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <set>
#include <string>
#include <string_view>
#include <unordered_set>

#include "main/lint_rules.h"
#include "utils/ast_utils.h"
#include "utils/location_utils.h"
#include "utils/name_utils.h"

namespace SL = SURELOG;

namespace {

auto CollectStructMembers(const SL::FileContent* fileContent,
                          SL::NodeId structNode) -> std::set<std::string_view> {
  std::set<std::string_view> members;
  SL::NodeId const kDataType = fileContent->Parent(structNode);
  if (!kDataType) {
    return members;
  }
  for (const auto& kMember : fileContent->sl_collect_all(
           kDataType, SL::VObjectType::paStruct_union_member)) {
    for (const auto& kVda : fileContent->sl_collect_all(
             kMember, SL::VObjectType::paVariable_decl_assignment)) {
      SL::NodeId const kName = fileContent->Child(kVda);
      if (kName && fileContent->Type(kName) == SL::VObjectType::slStringConst) {
        members.insert(fileContent->SymName(kName));
      }
    }
  }
  return members;
}

auto FindStructByTypeName(const SL::FileContent* fileContent, SL::NodeId root,
                          std::string_view typeName) -> SL::NodeId {
  for (const auto& kTypeDecl :
       fileContent->sl_collect_all(root, SL::VObjectType::paType_declaration)) {
    auto structs =
        fileContent->sl_collect_all(kTypeDecl, SL::VObjectType::paStruct_union);
    if (structs.empty()) {
      continue;
    }
    for (SL::NodeId child = fileContent->Child(kTypeDecl); child;
         child = fileContent->Sibling(child)) {
      if (fileContent->Type(child) == SL::VObjectType::slStringConst &&
          fileContent->SymName(child) == typeName) {
        return structs.front();
      }
    }
  }
  return SL::InvalidNodeId;
}

auto ResolveStructNode(const SL::FileContent* fileContent,
                       SL::NodeId moduleRoot, SL::NodeId fileRoot,
                       std::string_view varName) -> SL::NodeId {
  for (const auto& kNetDecl : fileContent->sl_collect_all(
           moduleRoot, SL::VObjectType::paNet_declaration)) {
    bool nameMatch = false;
    for (const auto& kAssign : fileContent->sl_collect_all(
             kNetDecl, SL::VObjectType::paNet_decl_assignment)) {
      SL::NodeId const kName = fileContent->Child(kAssign);
      if (kName && fileContent->Type(kName) == SL::VObjectType::slStringConst &&
          fileContent->SymName(kName) == varName) {
        nameMatch = true;
        break;
      }
    }
    if (!nameMatch) {
      continue;
    }
    auto structs =
        fileContent->sl_collect_all(kNetDecl, SL::VObjectType::paStruct_union);
    if (!structs.empty()) {
      return structs.front();
    }
    SL::NodeId const kTypeName = fileContent->Child(kNetDecl);
    if (kTypeName &&
        fileContent->Type(kTypeName) == SL::VObjectType::slStringConst) {
      return FindStructByTypeName(fileContent, fileRoot,
                                  fileContent->SymName(kTypeName));
    }
  }

  for (const auto& kVarDecl : fileContent->sl_collect_all(
           moduleRoot, SL::VObjectType::paData_declaration)) {
    auto structs =
        fileContent->sl_collect_all(kVarDecl, SL::VObjectType::paStruct_union);
    bool isInlineStruct = !structs.empty();

    bool nameMatch = false;
    for (const auto& kAssign : fileContent->sl_collect_all(
             kVarDecl, SL::VObjectType::paNet_decl_assignment)) {
      SL::NodeId const kName = fileContent->Child(kAssign);
      if (kName && fileContent->Type(kName) == SL::VObjectType::slStringConst &&
          fileContent->SymName(kName) == varName) {
        nameMatch = true;
        break;
      }
    }
    if (!nameMatch) {
      for (const auto& kAssign : fileContent->sl_collect_all(
               kVarDecl, SL::VObjectType::paVariable_decl_assignment)) {
        SL::NodeId p = fileContent->Parent(kAssign);
        if (p && fileContent->Type(p) ==
                     SL::VObjectType::paList_of_variable_decl_assignments) {
          SL::NodeId pp = fileContent->Parent(p);
          if (pp &&
              fileContent->Type(pp) == SL::VObjectType::paStruct_union_member) {
          }
        }
        SL::NodeId const kName = fileContent->Child(kAssign);
        if (kName &&
            fileContent->Type(kName) == SL::VObjectType::slStringConst &&
            fileContent->SymName(kName) == varName) {
          nameMatch = true;
          break;
        }
      }
    }
    if (!nameMatch) {
      continue;
    }
    if (isInlineStruct) {
      return structs.front();
    }
    SL::NodeId const kDataType = fileContent->Child(kVarDecl);
    if (!kDataType) {
      continue;
    }
    SL::NodeId const kTypeNameNode = fileContent->Child(kDataType);
    if (kTypeNameNode &&
        fileContent->Type(kTypeNameNode) == SL::VObjectType::slStringConst) {
      return FindStructByTypeName(fileContent, fileRoot,
                                  fileContent->SymName(kTypeNameNode));
    }
  }

  return SL::InvalidNodeId;
}

auto HasNonMemberKey(const SL::FileContent* fileContent,
                     SL::NodeId patNode) -> bool {
  for (const auto& kKey : fileContent->sl_collect_all(
           patNode, SL::VObjectType::paStructure_pattern_key)) {
    SL::NodeId const kChild = fileContent->Child(kKey);
    if (!kChild ||
        fileContent->Type(kChild) != SL::VObjectType::slStringConst) {
      return true;
    }
  }
  return false;
}

auto CollectPatternKeys(const SL::FileContent* fileContent, SL::NodeId patNode)
    -> std::unordered_set<std::string_view> {
  std::unordered_set<std::string_view> keys;
  for (const auto& kKey : fileContent->sl_collect_all(
           patNode, SL::VObjectType::paStructure_pattern_key)) {
    SL::NodeId const kName = fileContent->Child(kKey);
    if (kName && fileContent->Type(kName) == SL::VObjectType::slStringConst) {
      keys.insert(fileContent->SymName(kName));
    }
  }
  return keys;
}

}  // namespace

void CheckIncompleteAssignmentPattern(const SL::FileContent* fileContent,
                                      SL::ErrorContainer* errors,
                                      SL::SymbolTable* symbols) {
  if (fileContent == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }
  SL::NodeId const kRoot = fileContent->getRootNode();
  if (!kRoot) {
    return;
  }

  for (const auto& kPat : fileContent->sl_collect_all(
           kRoot, SL::VObjectType::paAssignment_pattern)) {
    if (!kPat) {
      continue;
    }

    if (fileContent
            ->sl_collect_all(kPat, SL::VObjectType::paStructure_pattern_key)
            .empty()) {
      continue;
    }

    if (HasNonMemberKey(fileContent, kPat)) {
      continue;
    }

    std::string_view const kVarName = FindDirectRhsLhsName(fileContent, kPat);
    if (kVarName == "<unknown>" || kVarName == "<indexed>") {
      continue;
    }

    SL::NodeId const kModuleRoot = FindEnclosingModule(fileContent, kPat);
    if (!kModuleRoot) {
      continue;
    }

    SL::NodeId const kStructNode =
        ResolveStructNode(fileContent, kModuleRoot, kRoot, kVarName);
    if (!kStructNode) {
      continue;
    }

    auto members = CollectStructMembers(fileContent, kStructNode);
    if (members.empty()) {
      continue;
    }

    auto provided = CollectPatternKeys(fileContent, kPat);

    std::string missing;
    for (std::string_view m : members) {
      if (!provided.contains(m)) {
        if (!missing.empty()) {
          missing += ", ";
        }
        missing += m;
      }
    }

    if (missing.empty()) {
      continue;
    }

    std::string sym =
        std::string(kVarName) + " is missing member(s): " + missing;
    ReportError(fileContent, kPat, sym,
                verihogg_lint::LINT_INCOMPLETE_ASSIGNMENT_PATTERN, errors,
                symbols);
  }
}