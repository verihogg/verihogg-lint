#include "utils/ast_utils.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/Design.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/Library/Library.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <algorithm>
#include <cstddef>
#include <gsl/span>
#include <initializer_list>
#include <sstream>
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace SL = SURELOG;

auto FindEnclosingModule(const SL::FileContent* fileContent, SL::NodeId node)
    -> SL::NodeId {
  SL::NodeId current = fileContent->Parent(node);
  while (current) {
    if (fileContent->Type(current) == SL::VObjectType::paModule_declaration) {
      return current;
    }
    current = fileContent->Parent(current);
  }
  return SL::InvalidNodeId;
}

auto HasSiblingOfType(const SL::FileContent* fileContent, SL::NodeId start,
                      SL::VObjectType type) -> bool {
  for (SL::NodeId tmp = fileContent->Sibling(start); tmp;
       tmp = fileContent->Sibling(tmp)) {
    if (fileContent->Type(tmp) == type) {
      return true;
    }
  }
  return false;
}

auto HasUnpackedDimension(const SL::FileContent* fileContent,
                          SL::NodeId varDecl) -> bool {
  return std::ranges::any_of(
      fileContent->sl_collect_all(varDecl,
                                  SL::VObjectType::paVariable_decl_assignment),
      [fileContent](SL::NodeId vda) -> bool {
        SL::NodeId const kNameNode = fileContent->Child(vda);
        if (!kNameNode) {
          return false;
        }
        return HasSiblingOfType(fileContent, kNameNode,
                                SL::VObjectType::paUnpacked_dimension) ||
               HasSiblingOfType(fileContent, kNameNode,
                                SL::VObjectType::paVariable_dimension);
      });
}

auto FindAncestorOfType(const SL::FileContent* fileContent, SL::NodeId node,
                        SL::VObjectType type) -> SL::NodeId {
  SL::NodeId current = fileContent->Parent(node);
  while (current) {
    if (fileContent->Type(current) == type) {
      return current;
    }
    current = fileContent->Parent(current);
  }
  return SL::InvalidNodeId;
}

auto FindSiblingOfType(const SL::FileContent* fileContent, SL::NodeId start,
                       SL::VObjectType type) -> SL::NodeId {
  for (SL::NodeId cur = fileContent->Sibling(start); cur;
       cur = fileContent->Sibling(cur)) {
    if (fileContent->Type(cur) == type) {
      return cur;
    }
  }
  return SL::InvalidNodeId;
}

auto FindChildOfType(const SL::FileContent* fileContent, SL::NodeId node,
                     SL::VObjectType type) -> SL::NodeId {
  for (SL::NodeId cur = fileContent->Child(node); cur;
       cur = fileContent->Sibling(cur)) {
    if (fileContent->Type(cur) == type) {
      return cur;
    }
  }
  return SL::InvalidNodeId;
}

auto IsBuiltinClass(const std::string& className) -> bool {
  return (className == "semaphore" || className == "process" ||
          className == "mailbox");
}

auto GetStringConst(const SL::FileContent* fileContent, SL::NodeId node)
    -> std::string {
  const SL::NodeId kStringId =
      fileContent->sl_get(node, SL::VObjectType::slStringConst);
  if (!kStringId) {
    return "";
  }
  const std::string kStr{fileContent->SymName(kStringId)};
  return kStr;
}

auto GetPrefix(const SL::FileContent* fileContent, SL::NodeId node)
    -> std::string {
  std::vector<std::string> contexts;
  const std::string libName{fileContent->getLibrary()->getName()};
  SL::NodeId tempId = fileContent->Parent(node);

  while (tempId) {
    SL::NodeId intermediateTempId;
    const SL::VObjectType type = fileContent->Type(tempId);

    switch (type) {
      case SL::VObjectType::paProgram_declaration: {
        intermediateTempId =
            fileContent->sl_get(tempId, SL::VObjectType::paProgram_ansi_header);
        break;
      }
      case SL::VObjectType::paInterface_declaration: {
        intermediateTempId = fileContent->sl_get(
            tempId, SL::VObjectType::paInterface_ansi_header);
        intermediateTempId = fileContent->sl_get(
            intermediateTempId, SL::VObjectType::paInterface_identifier);
        break;
      }
      case SL::VObjectType::paModule_declaration: {
        intermediateTempId =
            fileContent->sl_get(tempId, SL::VObjectType::paModule_ansi_header);
        break;
      }
      case SL::VObjectType::paClass_declaration:
      case SL::VObjectType::paPackage_declaration: {
        intermediateTempId =
            fileContent->sl_get(tempId, SL::VObjectType::slStringConst);
        break;
      }
      default: {
        tempId = fileContent->Parent(tempId);
        continue;
      }
    }

    const std::string stringConst =
        GetStringConst(fileContent, intermediateTempId);
    contexts.push_back(stringConst);
    tempId = fileContent->Parent(tempId);
  }

  std::string result;
  for (size_t i = 0; i < contexts.size(); i++) {
    if (i > 0) {
      result += "::";
    }
    result += contexts.at(i);
  }

  if (contexts.size() > 0) {
    result += "::";
  }

  return libName + "@" + result;
}

auto GetFullName(const SL::FileContent* fileContent, SL::NodeId node)
    -> std::string {
  const std::string kName = GetStringConst(fileContent, node);
  std::string kPrefix = GetPrefix(fileContent, node);
  if (kName == "") {
    return kPrefix;
  }
  return kPrefix + kName;
}

auto GetClassIds(const SL::FileContent* fileContent)
    -> std::unordered_map<std::string, SL::NodeId> {
  const std::vector<SL::NodeId> kClassDeclarations =
      fileContent->sl_collect_all(fileContent->getRootNode(),
                                  SL::VObjectType::paClass_declaration);
  std::unordered_map<std::string, SL::NodeId> classes;

  for (const SL::NodeId classId : kClassDeclarations) {
    const std::string kClassName = GetFullName(fileContent, classId);
    if (IsBuiltinClass(kClassName)) {
      continue;
    }
    classes[kClassName] = classId;
  }
  return classes;
}

auto GetInterfaceClassSet(const SL::FileContent* fileContent)
    -> std::unordered_set<std::string> {
  const std::vector<SL::NodeId> kInterfaceClassDeclarations =
      fileContent->sl_collect_all(
          fileContent->getRootNode(),
          SL::VObjectType::paInterface_class_declaration);
  std::unordered_set<std::string> interfaceClassSet;
  for (const auto& interfaceClass : kInterfaceClassDeclarations) {
    const std::string interfaceClassName =
        GetStringConst(fileContent, interfaceClass);
    interfaceClassSet.insert(interfaceClassName);
  }
  return interfaceClassSet;
}

auto GetClassSet(const SL::FileContent* fileContent)
    -> std::unordered_set<std::string> {
  const std::vector<SL::NodeId> kClassDeclarations =
      fileContent->sl_collect_all(fileContent->getRootNode(),
                                  SL::VObjectType::paClass_declaration);
  std::unordered_set<std::string> classSet;
  for (const auto& classId : kClassDeclarations) {
    const std::string className = GetStringConst(fileContent, classId);
    classSet.insert(className);
  }
  return classSet;
}

auto GetFullNameFromScope(const SL::FileContent* fileContent, SL::NodeId node)
    -> std::string {
  std::stringstream sstream;
  sstream << GetPrefix(fileContent, node);

  const SL::NodeId kTempId =
      fileContent->sl_get(node, SL::VObjectType::paClass_type);
  const std::vector<SL::NodeId> kStrIds =
      fileContent->sl_collect_all(kTempId, SL::VObjectType::slStringConst);

  const std::string firstString{fileContent->SymName(kStrIds.at(0))};
  sstream << firstString;

  for (auto kStringId : gsl::span{kStrIds}.subspan(1)) {
    const std::string scopeName{fileContent->SymName(kStringId)};
    sstream << "::" << scopeName;
  }
  return sstream.str();
}

auto GetSuperclassString(const SL::FileContent* fileContent, SL::NodeId node)
    -> std::string {
  const SL::NodeId kClassType =
      fileContent->sl_get(node, SL::VObjectType::paClass_type);
  if (!kClassType) {
    return "";
  }
  return GetStringConst(fileContent, kClassType);
}
auto SubtreeContainsAnyType(const SL::FileContent* fileContent, SL::NodeId root,
                            std::initializer_list<SL::VObjectType> targetTypes,
                            NodePrunePredicate shouldPrune) -> bool {
  if (fileContent == nullptr || !root) {
    return false;
  }

  std::stack<SL::NodeId> worklist;
  worklist.push(root);

  while (!worklist.empty()) {
    SL::NodeId const current = worklist.top();
    worklist.pop();

    SL::VObjectType const type = fileContent->Type(current);

    if (shouldPrune != nullptr && shouldPrune(fileContent, current, type)) {
      continue;
    }

    if (std::ranges::find(targetTypes, type) != targetTypes.end()) {
      return true;
    }

    for (SL::NodeId child = fileContent->Child(current); child;
         child = fileContent->Sibling(child)) {
      worklist.push(child);
    }
  }

  return false;
}

auto FindVarOrNetDecl(const SL::FileContent* fc, SL::NodeId root,
                      std::string_view name) -> SL::NodeId {
  if (fc == nullptr || !root || name.empty()) {
    return SL::InvalidNodeId;
  }

  for (SL::NodeId const decl :
       fc->sl_collect_all(root, SL::VObjectType::paVariable_declaration)) {
    for (SL::NodeId const assign : fc->sl_collect_all(
             decl, SL::VObjectType::paVariable_decl_assignment)) {
      SL::NodeId const nameNode = fc->Child(assign);
      if (nameNode && fc->Type(nameNode) == SL::VObjectType::slStringConst &&
          fc->SymName(nameNode) == name) {
        return decl;
      }
    }
  }
  for (SL::NodeId const decl :
       fc->sl_collect_all(root, SL::VObjectType::paNet_declaration)) {
    for (SL::NodeId const assign :
         fc->sl_collect_all(decl, SL::VObjectType::paNet_decl_assignment)) {
      SL::NodeId const nameNode = fc->Child(assign);
      if (nameNode && fc->Type(nameNode) == SL::VObjectType::slStringConst &&
          fc->SymName(nameNode) == name) {
        return decl;
      }
    }
  }

  return SL::InvalidNodeId;
}

auto GetDataType(const SL::FileContent* fc, SL::NodeId declNode) -> SL::NodeId {
  if (fc == nullptr || !declNode) {
    return SL::InvalidNodeId;
  }

  for (SL::NodeId cur = fc->Child(declNode); cur; cur = fc->Sibling(cur)) {
    if (fc->Type(cur) == SL::VObjectType::paData_type) {
      return cur;
    }
  }

  return SL::InvalidNodeId;
}

auto GetTypedefName(const SL::FileContent* fc, SL::NodeId declNode)
    -> std::string_view {
  if (fc == nullptr || !declNode) {
    return "";
  }

  if (fc->Type(declNode) == SL::VObjectType::paNet_declaration) {
    SL::NodeId const first = fc->Child(declNode);
    if (first && fc->Type(first) == SL::VObjectType::slStringConst) {
      return fc->SymName(first);
    }
    return "";
  }

  SL::NodeId const dataType = GetDataType(fc, declNode);
  if (!dataType) {
    return "";
  }

  SL::NodeId const child = fc->Child(dataType);
  if (child && fc->Type(child) == SL::VObjectType::slStringConst) {
    return fc->SymName(child);
  }

  return "";
}

auto IsModuleOrInterfaceInstance(const SL::FileContent* fc, SL::NodeId root,
                                 std::string_view varName) -> bool {
  if (fc == nullptr || !root || varName.empty()) {
    return false;
  }

  for (SL::NodeId const hierInst :
       fc->sl_collect_all(root, SL::VObjectType::paHierarchical_instance)) {
    SL::NodeId const nameOfInst = fc->Child(hierInst);
    if (!nameOfInst) {
      continue;
    }

    SL::NodeId const instName = fc->Child(nameOfInst);
    if (!instName || fc->Type(instName) != SL::VObjectType::slStringConst) {
      continue;
    }

    if (fc->SymName(instName) == varName) {
      return true;
    }
  }

  return false;
}
