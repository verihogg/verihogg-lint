#include "utils/class_scope_utils.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <array>
#include <string_view>
#include <unordered_map>

namespace SL = SURELOG;

namespace ClassScopeUtils {

namespace {
constexpr std::array kScopeContainerTypes = {
    SL::VObjectType::paSource_text,
    SL::VObjectType::paPackage_declaration,
    SL::VObjectType::paModule_declaration,
};
}  // namespace

auto FindScopeContainer(const SL::FileContent* fileContent, SL::NodeId node)
    -> SL::NodeId {
  SL::NodeId current = fileContent->Parent(node);
  while (current) {
    for (auto const kType : kScopeContainerTypes) {
      if (fileContent->Type(current) == kType) {
        return current;
      }
    }
    current = fileContent->Parent(current);
  }
  return SL::InvalidNodeId;
}

auto GetScopeContainerName(const SL::FileContent* fileContent,
                           SL::NodeId scopeContainer) -> std::string_view {
  SL::VObjectType const kType = fileContent->Type(scopeContainer);

  if (kType == SL::VObjectType::paSource_text) {
    return "";
  }

  if (kType == SL::VObjectType::paPackage_declaration) {
    SL::NodeId const kPkgKw = fileContent->Child(scopeContainer);
    if (!kPkgKw) {
      return "";
    }
    SL::NodeId const kNameNode = fileContent->Sibling(kPkgKw);
    if (!kNameNode ||
        fileContent->Type(kNameNode) != SL::VObjectType::slStringConst) {
      return "";
    }
    return fileContent->SymName(kNameNode);
  }

  if (kType == SL::VObjectType::paModule_declaration) {
    SL::NodeId const kHeader = fileContent->Child(scopeContainer);
    if (!kHeader) {
      return "";
    }
    SL::NodeId const kEyword = fileContent->Child(kHeader);
    if (!kEyword) {
      return "";
    }
    SL::NodeId const kNameNode = fileContent->Sibling(kEyword);
    if (!kNameNode ||
        fileContent->Type(kNameNode) != SL::VObjectType::slStringConst) {
      return "";
    }
    return fileContent->SymName(kNameNode);
  }

  return "";
}

auto BuildClassScopeMap(const SL::FileContent* fileContent)
    -> std::unordered_map<std::string_view, ClassScopeInfo> {
  std::unordered_map<std::string_view, ClassScopeInfo> result;

  SL::NodeId const kRoot = fileContent->getRootNode();
  if (!kRoot) {
    return result;
  }

  for (SL::NodeId const kClassDecl : fileContent->sl_collect_all(
           kRoot, SL::VObjectType::paClass_declaration)) {
    SL::NodeId const kClassKw = fileContent->Child(kClassDecl);
    if (!kClassKw || fileContent->Type(kClassKw) != SL::VObjectType::paCLASS) {
      continue;
    }
    SL::NodeId const kClassNameNode = fileContent->Sibling(kClassKw);
    if (!kClassNameNode ||
        fileContent->Type(kClassNameNode) != SL::VObjectType::slStringConst) {
      continue;
    }
    std::string_view const kClassName = fileContent->SymName(kClassNameNode);

    SL::NodeId const kScopeContainer =
        FindScopeContainer(fileContent, kClassDecl);
    if (!kScopeContainer) {
      continue;
    }

    result[kClassName] = ClassScopeInfo{
        .scopeType = fileContent->Type(kScopeContainer),
        .scopeName = GetScopeContainerName(fileContent, kScopeContainer),
    };
  }

  return result;
}

auto ScopesMatch(const ClassScopeInfo& implScope,
                 const ClassScopeInfo& classScope) -> bool {
  if (implScope.scopeType != classScope.scopeType) {
    return false;
  }
  if (implScope.scopeType == SL::VObjectType::paSource_text) {
    return true;
  }
  return implScope.scopeName == classScope.scopeName;
}

}  // namespace ClassScopeUtils