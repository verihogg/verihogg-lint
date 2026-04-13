#include "utils/class_scope_utils.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <array>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "utils/ast_utils.h"

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
    SL::NodeId const kKeyword = fileContent->Child(kHeader);
    if (!kKeyword) {
      return "";
    }
    SL::NodeId const kNameNode = fileContent->Sibling(kKeyword);
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

auto CollectClassDeclInfos(const SL::FileContent* fileContent)
    -> std::vector<ClassDeclInfo> {
  std::vector<ClassDeclInfo> result;

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

    SL::NodeId const kScopeContainer =
        FindScopeContainer(fileContent, kClassDecl);
    if (!kScopeContainer) {
      continue;
    }

    result.push_back(ClassDeclInfo{
        .className = fileContent->SymName(kClassNameNode),
        .scopeInfo =
            ClassScopeInfo{
                .scopeType = fileContent->Type(kScopeContainer),
                .scopeName =
                    GetScopeContainerName(fileContent, kScopeContainer),
            },
    });
  }

  return result;
}

auto ExtractClassScopedMember(const SL::FileContent* fileContent,
                              SL::NodeId node)
    -> std::optional<ClassScopedMemberInfo> {
  SL::NodeId const kClassScope =
      FindChildOfType(fileContent, node, SL::VObjectType::paClass_scope);
  if (!kClassScope) {
    return std::nullopt;
  }

  SL::NodeId const kClassType = fileContent->Child(kClassScope);
  if (!kClassType ||
      fileContent->Type(kClassType) != SL::VObjectType::paClass_type) {
    return std::nullopt;
  }

  SL::NodeId classNameNode = SL::InvalidNodeId;
  for (SL::NodeId cur = fileContent->Child(kClassType); cur;
       cur = fileContent->Sibling(cur)) {
    if (fileContent->Type(cur) == SL::VObjectType::slStringConst) {
      classNameNode = cur;
    }
  }
  if (!classNameNode) {
    return std::nullopt;
  }

  SL::NodeId const kMemberNameNode = fileContent->Sibling(kClassScope);
  if (!kMemberNameNode ||
      fileContent->Type(kMemberNameNode) != SL::VObjectType::slStringConst) {
    return std::nullopt;
  }

  return ClassScopedMemberInfo{
      .className = fileContent->SymName(classNameNode),
      .memberName = fileContent->SymName(kMemberNameNode),
      .classScopeNode = kClassScope,
  };
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

auto MakeClassMethodKey(std::string_view className, std::string_view funcName)
    -> std::string {
  std::string key;
  key.reserve(className.size() + 2 + funcName.size());
  key.append(className);
  key.append("::");
  key.append(funcName);
  return key;
}

auto GetClassNameFromDecl(const SL::FileContent* fc, SL::NodeId classDecl)
    -> std::string_view {
  SL::NodeId const kClassKw = fc->Child(classDecl);
  if (!kClassKw || fc->Type(kClassKw) != SL::VObjectType::paCLASS) {
    return {};
  }
  SL::NodeId const kNameNode = fc->Sibling(kClassKw);
  if (!kNameNode || fc->Type(kNameNode) != SL::VObjectType::slStringConst) {
    return {};
  }
  return fc->SymName(kNameNode);
}

auto ExtractFuncNameFromPrototype(const SL::FileContent* fc,
                                  SL::NodeId funcProto) -> std::string_view {
  SL::NodeId const kFdtoi = fc->Child(funcProto);
  if (!kFdtoi ||
      fc->Type(kFdtoi) != SL::VObjectType::paFunction_data_type_or_implicit) {
    return {};
  }
  SL::NodeId const kNameNode = fc->Sibling(kFdtoi);
  if (!kNameNode || fc->Type(kNameNode) != SL::VObjectType::slStringConst) {
    return {};
  }
  return fc->SymName(kNameNode);
}

}  // namespace ClassScopeUtils