#include "utils/ast_utils.h"

#include <algorithm>
#include <sstream>
#include <unordered_set>

#include "Surelog/Library/Library.h"

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

auto isBuiltinClass(const std::string& className) -> bool {
  return (className == "semaphore" || className == "process" ||
          className == "mailbox");
}

auto getStringConst(const SL::FileContent* fC, SL::NodeId id) -> std::string {
  NodeId kStringId = fC->sl_get(id, VObjectType::slStringConst);
  if (kStringId == zeroId) {
    return "";
  }
  std::string kStr{fC->SymName(kStringId)};
  return kStr;
}

auto getPrefix(const SL::FileContent* fC, SL::NodeId id) -> std::string {
  std::vector<std::string> contexts;
  std::string libName{fC->getLibrary()->getName()};
  SL::NodeId tempId = fC->Parent(id);

  while (tempId) {
    SL::NodeId intermediateTempId;
    SL::VObjectType type = fC->Type(tempId);

    switch (type) {
      case SL::VObjectType::paProgram_declaration: {
        intermediateTempId =
            fC->sl_get(tempId, SL::VObjectType::paProgram_ansi_header);
        break;
      }
      case SL::VObjectType::paInterface_declaration: {
        intermediateTempId =
            fC->sl_get(tempId, SL::VObjectType::paInterface_ansi_header);
        intermediateTempId = fC->sl_get(
            intermediateTempId, SL::VObjectType::paInterface_identifier);
        break;
      }
      case SL::VObjectType::paModule_declaration: {
        intermediateTempId =
            fC->sl_get(tempId, SL::VObjectType::paModule_ansi_header);
        break;
      }
      case SL::VObjectType::paClass_declaration:
      case SL::VObjectType::paPackage_declaration: {
        intermediateTempId = fC->sl_get(tempId, SL::VObjectType::slStringConst);
        break;
      }
      default: {
        tempId = fC->Parent(tempId);
        continue;
      }
    }

    std::string stringConst = getStringConst(fC, intermediateTempId);
    contexts.push_back(stringConst);
    tempId = fC->Parent(tempId);
  }

  std::string result;
  for (size_t i = 0; i < contexts.size(); i++) {
    if (i > 0) {
      result += "::";
    }
    result += contexts[i];
  }

  if (contexts.size() > 0) {
    result += "::";
  }

  return libName + "@" + result;
}

auto getFullName(const SL::FileContent* fC, SL::NodeId id) -> std::string {
  const std::string kName = getStringConst(fC, id);
  std::string kPrefix = getPrefix(fC, id);
  if (kName == "") {
    return kPrefix;
  }
  return kPrefix + kName;
}

auto getClassIds(const SL::FileContent* fC)
    -> std::unordered_map<std::string, SL::NodeId> {
  const std::vector<SL::NodeId> classDeclarations = fC->sl_collect_all(
      fC->getRootNode(), SL::VObjectType::paClass_declaration);
  std::unordered_map<std::string, SL::NodeId> classes;

  for (NodeId classId : classDeclarations) {
    const std::string className = getFullName(fC, classId);
    if (isBuiltinClass(className)) continue;
    classes[className] = classId;
  }
  return classes;
}

auto removeFilePrefix(std::string str) -> std::string {
  size_t ind = 0;
  while (str[ind++] != '@') {
  }
  return std::string(str).substr(ind, str.size());
}

auto getClassScope(const SL::FileContent* fC, SL::NodeId funcBodyId)
    -> std::vector<std::string> {
  std::vector<std::string> scopes;
  const SL::NodeId classScopeId =
      fC->sl_collect(funcBodyId, SL::VObjectType::paClass_scope);
  const SL::NodeId classTypeId =
      fC->sl_get(classScopeId, SL::VObjectType::paClass_type);

  if (classTypeId == zeroId) {
    return scopes;
  }
  const std::vector<SL::NodeId> kIds =
      fC->sl_collect_all(funcBodyId, SL::VObjectType::slStringConst);

  for (auto& kId : kIds) {
    std::string kStr{fC->SymName(kId)};
    scopes.push_back(kStr);
  }

  return scopes;
}

auto getInterfaceClassSet(const SL::FileContent* fC)
    -> std::unordered_set<std::string> {
  const std::vector<SL::NodeId> kInterfaceClassDeclarations =
      fC->sl_collect_all(fC->getRootNode(),
                         SL::VObjectType::paInterface_class_declaration);
  std::unordered_set<std::string> interfaceClassSet;
  for (auto& interfaceClass : kInterfaceClassDeclarations) {
    std::string interfaceClassName = getStringConst(fC, interfaceClass);
    interfaceClassSet.insert(interfaceClassName);
  }
  return interfaceClassSet;
}

auto getClassSet(const SL::FileContent* fC) -> std::unordered_set<std::string> {
  const std::vector<SL::NodeId> kClassDeclarations = fC->sl_collect_all(
      fC->getRootNode(), SL::VObjectType::paClass_declaration);
  std::unordered_set<std::string> classSet;
  for (auto& classId : kClassDeclarations) {
    std::string className = getStringConst(fC, classId);
    classSet.insert(className);
  }
  return classSet;
}

auto getFullNameFromScope(const SL::FileContent* fC, SL::NodeId id)
    -> std::string {
  std::stringstream sstream;
  std::string prefix = getPrefix(fC, id);
  sstream << prefix;

  const NodeId tempId = fC->sl_get(id, VObjectType::paClass_type);
  const std::vector<NodeId> strIds =
      fC->sl_collect_all(tempId, VObjectType::slStringConst);
  assert(strIds.size() > 0);

  std::string firstString{fC->SymName(strIds[0])};
  sstream << firstString;

  for (size_t i = 1; i < strIds.size(); i++) {
    const NodeId stringId = strIds[i];
    std::string scopeName{fC->SymName(stringId)};
    sstream << "::" << scopeName;
  }
  return sstream.str();
}

std::string getSuperclassString(const FileContent* fC, NodeId id) {
  assert(fC->Type(id) != VObjectType::paClass_declaration);

  const NodeId classType = fC->sl_get(id, VObjectType::paClass_type);
  if (classType == zeroId) return "";
  return getStringConst(fC, classType);
}
