#include "utils/ast_utils.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/Design.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/Library/Library.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <gsl/span>
#include <initializer_list>
#include <optional>
#include <sstream>
#include <stack>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "utils/design_utils.h"

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

auto GetClassIds(SL::Design* design)
    -> std::unordered_map<std::string, SL::NodeId> {
  std::unordered_map<std::string, SL::NodeId> classes;

  DesignUtils::ForEachFileContent(
      design, [&](const SL::FileContent* fileContent) {
        const std::vector<SL::NodeId> kClassDeclarations =
            fileContent->sl_collect_all(fileContent->getRootNode(),
                                        SL::VObjectType::paClass_declaration);

        for (const SL::NodeId classId : kClassDeclarations) {
          const std::string kClassName = GetFullName(fileContent, classId);
          if (IsBuiltinClass(kClassName)) {
            continue;
          }
          classes[kClassName] = classId;
        }
      });
  return classes;
}

auto GetClassDeclByName(const SL::FileContent* fileContent,
                        std::string_view className) -> SL::NodeId {
  if (fileContent == nullptr || className.empty()) {
    return SL::InvalidNodeId;
  }
  SL::NodeId const kRoot = fileContent->getRootNode();
  for (SL::NodeId const kClassDecl : fileContent->sl_collect_all(
           kRoot, SL::VObjectType::paClass_declaration)) {
    if (GetStringConst(fileContent, kClassDecl) == className) {
      return kClassDecl;
    }
  }
  return SL::InvalidNodeId;
}

auto GetInterfaceClassSet(SL::Design* design)
    -> std::unordered_set<std::string> {
  std::unordered_set<std::string> interfaceClassSet;
  DesignUtils::ForEachFileContent(
      design, [&](const SL::FileContent* fileContent) {
        const std::vector<SL::NodeId> kInterfaceClassDeclarations =
            fileContent->sl_collect_all(
                fileContent->getRootNode(),
                SL::VObjectType::paInterface_class_declaration);

        for (const auto& interfaceClass : kInterfaceClassDeclarations) {
          const std::string interfaceClassName =
              GetStringConst(fileContent, interfaceClass);
          interfaceClassSet.insert(interfaceClassName);
        }
      });
  return interfaceClassSet;
}

auto GetClassSetFromFileContent(const SL::FileContent* fileContent)
    -> std::unordered_set<std::string> {
  std::unordered_set<std::string> classSet;
  const std::vector<SL::NodeId> kClassDeclarations =
      fileContent->sl_collect_all(fileContent->getRootNode(),
                                  SL::VObjectType::paClass_declaration);

  for (const auto& classId : kClassDeclarations) {
    const std::string className = GetStringConst(fileContent, classId);
    classSet.insert(className);
  }
  return classSet;
}

auto GetClassSet(SL::Design* design) -> std::unordered_set<std::string> {
  std::unordered_set<std::string> classSet;
  DesignUtils::ForEachFileContent(
      design, [&](const SL::FileContent* fileContent) {
        const std::vector<SL::NodeId> kClassDeclarations =
            fileContent->sl_collect_all(fileContent->getRootNode(),
                                        SL::VObjectType::paClass_declaration);

        for (const auto& classId : kClassDeclarations) {
          const std::string className = GetStringConst(fileContent, classId);
          classSet.insert(className);
        }
      });
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

auto Is1BitScalarKeyword(SL::VObjectType type) -> bool {
  static constexpr std::array kScalarTypes = {
      SL::VObjectType::paIntVec_TypeBit,
      SL::VObjectType::paIntVec_TypeLogic,
      SL::VObjectType::paIntVec_TypeReg,
  };
  return std::ranges::any_of(
      kScalarTypes, [type](SL::VObjectType t) -> bool { return t == type; });
}

auto IsIntegralType(SL::VObjectType type) -> bool {
  static constexpr std::array kIntegralTypes = {
      SL::VObjectType::paIntVec_TypeBit,
      SL::VObjectType::paIntVec_TypeLogic,
      SL::VObjectType::paIntVec_TypeReg,
      SL::VObjectType::paIntegerAtomType_Int,
      SL::VObjectType::paIntegerAtomType_LongInt,
      SL::VObjectType::paIntegerAtomType_Shortint,
      SL::VObjectType::paIntegerAtomType_Byte,
      SL::VObjectType::paIntegerAtomType_Integer,
      SL::VObjectType::paIntegerAtomType_Time,
      SL::VObjectType::paEnum_base_type,
  };
  return std::ranges::find(kIntegralTypes, type) != kIntegralTypes.end();
}

auto IsWildcardNumberType(SL::VObjectType type) -> bool {
  switch (type) {
    case SL::VObjectType::paNumber_1Tickbx:
    case SL::VObjectType::paNumber_1Tickb0:
    case SL::VObjectType::paNumber_1TickbX:
      return true;
    default:
      return false;
  }
}

auto IsOperatorType(SL::VObjectType type) -> bool {
  using UnderlyingType = std::underlying_type_t<SL::VObjectType>;
  const auto kVal = static_cast<UnderlyingType>(type);
  return kVal >= static_cast<UnderlyingType>(SL::VObjectType::paUnary_Minus) &&
         kVal <= static_cast<UnderlyingType>(SL::VObjectType::paUnary_Tilda);
}

auto CollectUserDefinedTypes(const SL::FileContent* fileContent,
                             SL::NodeId root)
    -> std::unordered_set<std::string_view> {
  std::unordered_set<std::string_view> userTypes;
  for (SL::NodeId const kDeclNode :
       fileContent->sl_collect_all(root, SL::VObjectType::paType_declaration)) {
    bool seenDataType = false;
    for (SL::NodeId cur = fileContent->Child(kDeclNode); cur;
         cur = fileContent->Sibling(cur)) {
      const SL::VObjectType kChildType = fileContent->Type(cur);
      if (kChildType == SL::VObjectType::paData_type) {
        seenDataType = true;
      } else if (kChildType == SL::VObjectType::slStringConst && seenDataType) {
        std::string_view const kTypeName = fileContent->SymName(cur);
        if (!kTypeName.empty()) {
          userTypes.insert(kTypeName);
        }
      }
    }
  }
  return userTypes;
}

auto ExtractTypeInfoFromDataType(const SL::FileContent* fc, SL::NodeId dataType)
    -> std::optional<SvTypeInfo> {
  if (!dataType || fc->Type(dataType) != SL::VObjectType::paData_type) {
    return std::nullopt;
  }

  SL::NodeId const kTypeChild = fc->Child(dataType);
  if (!kTypeChild) {
    return std::nullopt;
  }

  SL::VObjectType kNodeType = fc->Type(kTypeChild);
  SL::NodeId kNameNode = kTypeChild;

  if (kNodeType == SL::VObjectType::paClass_scope) {
    SL::NodeId const kScopedName = fc->Sibling(kTypeChild);
    if (kScopedName &&
        fc->Type(kScopedName) == SL::VObjectType::slStringConst) {
      kNodeType = SL::VObjectType::slStringConst;
      kNameNode = kScopedName;
    }
  }

  std::string_view name;
  if (kNodeType == SL::VObjectType::slStringConst) {
    name = fc->SymName(kNameNode);
  }

  return SvTypeInfo{.nodeType = kNodeType, .name = name};
}

auto SvTypeInfosMatch(const SvTypeInfo& a, const SvTypeInfo& b) -> bool {
  if (a.nodeType != b.nodeType) {
    return false;
  }
  if (a.nodeType == SL::VObjectType::slStringConst) {
    return a.name == b.name;
  }
  return true;
}

auto ExtractArgTypesFromPortList(const SL::FileContent* fc, SL::NodeId portList)
    -> std::vector<SvTypeInfo> {
  std::vector<SvTypeInfo> result;
  if (!portList) {
    return result;
  }

  for (SL::NodeId item = fc->Child(portList); item; item = fc->Sibling(item)) {
    if (fc->Type(item) != SL::VObjectType::paTf_port_item) {
      continue;
    }

    SL::NodeId kDtoi = SL::InvalidNodeId;
    for (SL::NodeId child = fc->Child(item); child;
         child = fc->Sibling(child)) {
      if (fc->Type(child) == SL::VObjectType::paData_type_or_implicit) {
        kDtoi = child;
        break;
      }
    }
    if (!kDtoi) {
      continue;
    }

    SL::NodeId const kDataType = fc->Child(kDtoi);
    if (!kDataType || fc->Type(kDataType) != SL::VObjectType::paData_type) {
      continue;
    }

    auto info = ExtractTypeInfoFromDataType(fc, kDataType);
    if (info.has_value()) {
      result.push_back(*info);
    }
  }
  return result;
}
