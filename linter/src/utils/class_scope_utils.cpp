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
static constexpr std::array kScopeContainerTypes = {
    SL::VObjectType::paSource_text,
    SL::VObjectType::paPackage_declaration,
    SL::VObjectType::paModule_declaration,
};
}  // namespace

auto FindScopeContainer(const SL::FileContent* fileContent, SL::NodeId node)
    -> SL::NodeId {
  SL::NodeId current = fileContent->Parent(node);
  while (current) {
    for (auto const type : kScopeContainerTypes) {
      if (fileContent->Type(current) == type) {
        return current;
      }
    }
    current = fileContent->Parent(current);
  }
  return SL::InvalidNodeId;
}

auto GetScopeContainerName(const SL::FileContent* fileContent,
                           SL::NodeId scopeContainer) -> std::string_view {
  SL::VObjectType const type = fileContent->Type(scopeContainer);

  if (type == SL::VObjectType::paSource_text) {
    return "";
  }

  if (type == SL::VObjectType::paPackage_declaration) {
    SL::NodeId const pkgKw = fileContent->Child(scopeContainer);
    if (!pkgKw) {
      return "";
    }
    SL::NodeId const nameNode = fileContent->Sibling(pkgKw);
    if (!nameNode ||
        fileContent->Type(nameNode) != SL::VObjectType::slStringConst) {
      return "";
    }
    return fileContent->SymName(nameNode);
  }

  if (type == SL::VObjectType::paModule_declaration) {
    SL::NodeId const header = fileContent->Child(scopeContainer);
    if (!header) {
      return "";
    }
    SL::NodeId const keyword = fileContent->Child(header);
    if (!keyword) {
      return "";
    }
    SL::NodeId const nameNode = fileContent->Sibling(keyword);
    if (!nameNode ||
        fileContent->Type(nameNode) != SL::VObjectType::slStringConst) {
      return "";
    }
    return fileContent->SymName(nameNode);
  }

  return "";
}

auto BuildClassScopeMap(const SL::FileContent* fileContent)
    -> std::unordered_map<std::string_view, ClassScopeInfo> {
  std::unordered_map<std::string_view, ClassScopeInfo> result;

  SL::NodeId const root = fileContent->getRootNode();
  if (!root) {
    return result;
  }

  for (SL::NodeId const classDecl : fileContent->sl_collect_all(
           root, SL::VObjectType::paClass_declaration)) {
    SL::NodeId const classKw = fileContent->Child(classDecl);
    if (!classKw || fileContent->Type(classKw) != SL::VObjectType::paCLASS) {
      continue;
    }
    SL::NodeId const classNameNode = fileContent->Sibling(classKw);
    if (!classNameNode ||
        fileContent->Type(classNameNode) != SL::VObjectType::slStringConst) {
      continue;
    }
    std::string_view const className = fileContent->SymName(classNameNode);

    SL::NodeId const scopeContainer =
        FindScopeContainer(fileContent, classDecl);
    if (!scopeContainer) {
      continue;
    }

    result[className] = ClassScopeInfo{
        .scopeType = fileContent->Type(scopeContainer),
        .scopeName = GetScopeContainerName(fileContent, scopeContainer),
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