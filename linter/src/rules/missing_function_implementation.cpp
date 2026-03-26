#include "rules/missing_function_implementation.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/Design.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/ErrorReporting/ErrorDefinition.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <optional>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

#include "main/lint_rules.h"
#include "utils/ast_utils.h"
#include "utils/design_utils.h"
#include "utils/location_utils.h"

namespace SL = SURELOG;

namespace {

auto MakeKey(std::string_view className, std::string_view methodName)
    -> std::string {
  std::string key;
  key.reserve(className.size() + 2 + methodName.size());
  key.append(className);
  key.append("::");
  key.append(methodName);
  return key;
}

auto ExtractMethodNameFromPrototype(const SL::FileContent* fileContent,
                                    SL::NodeId methodProto)
    -> std::string_view {
  SL::NodeId const kFirstChild = fileContent->Child(methodProto);
  if (!kFirstChild) {
    return {};
  }

  SL::VObjectType const kProtoType = fileContent->Type(methodProto);

  if (kProtoType == SL::VObjectType::paFunction_prototype) {
    if (fileContent->Type(kFirstChild) !=
        SL::VObjectType::paFunction_data_type_or_implicit) {
      return {};
    }
    SL::NodeId const kNameNode = fileContent->Sibling(kFirstChild);
    if (!kNameNode ||
        fileContent->Type(kNameNode) != SL::VObjectType::slStringConst) {
      return {};
    }
    return fileContent->SymName(kNameNode);
  }

  if (kProtoType == SL::VObjectType::paTask_prototype) {
    if (fileContent->Type(kFirstChild) != SL::VObjectType::slStringConst) {
      return {};
    }
    return fileContent->SymName(kFirstChild);
  }

  return {};
}

struct ExternMethodInfo {
  std::string_view className;
  std::string_view methodName;
  SL::NodeId reportNode;
  const SL::FileContent* fileContent;
  bool isTask;
};

auto GetClassNameFromClassDeclaration(const SL::FileContent* fileContent,
                                      SL::NodeId classDecl)
    -> std::string_view {
  SL::NodeId const kClassKw = fileContent->Child(classDecl);
  if (!kClassKw || fileContent->Type(kClassKw) != SL::VObjectType::paCLASS) {
    return {};
  }

  SL::NodeId const kClassNameNode = fileContent->Sibling(kClassKw);
  if (!kClassNameNode ||
      fileContent->Type(kClassNameNode) != SL::VObjectType::slStringConst) {
    return {};
  }

  return fileContent->SymName(kClassNameNode);
}

auto TryCollectExternMethodInfo(const SL::FileContent* fileContent,
                                SL::NodeId classItem,
                                std::string_view className)
    -> std::optional<ExternMethodInfo> {
  SL::NodeId const kClassMethod = fileContent->Child(classItem);
  if (!kClassMethod ||
      fileContent->Type(kClassMethod) != SL::VObjectType::paClass_method) {
    return std::nullopt;
  }

  SL::NodeId const kFirstChild = fileContent->Child(kClassMethod);
  if (!kFirstChild ||
      fileContent->Type(kFirstChild) != SL::VObjectType::paExtern_qualifier) {
    return std::nullopt;
  }

  SL::NodeId const kMethodProtoContainer = FindSiblingOfType(
      fileContent, kFirstChild, SL::VObjectType::paMethod_prototype);
  if (!kMethodProtoContainer) {
    return std::nullopt;
  }

  SL::NodeId const kProto = fileContent->Child(kMethodProtoContainer);
  if (!kProto) {
    return std::nullopt;
  }

  SL::VObjectType const kProtoType = fileContent->Type(kProto);
  if (kProtoType != SL::VObjectType::paFunction_prototype &&
      kProtoType != SL::VObjectType::paTask_prototype) {
    return std::nullopt;
  }

  std::string_view const kMethodName =
      ExtractMethodNameFromPrototype(fileContent, kProto);
  if (kMethodName.empty()) {
    return std::nullopt;
  }

  return ExternMethodInfo{
      .className = className,
      .methodName = kMethodName,
      .reportNode = kMethodProtoContainer,
      .fileContent = fileContent,
      .isTask = (kProtoType == SL::VObjectType::paTask_prototype),
  };
}

auto CollectExternMethodsFromClassDeclaration(
    const SL::FileContent* fileContent, SL::NodeId classDecl,
    std::vector<ExternMethodInfo>& out) -> void {
  std::string_view const kClassName =
      GetClassNameFromClassDeclaration(fileContent, classDecl);
  if (kClassName.empty()) {
    return;
  }

  for (SL::NodeId const kClassItem :
       fileContent->sl_collect_all(classDecl, SL::VObjectType::paClass_item)) {
    std::optional<ExternMethodInfo> const kInfo =
        TryCollectExternMethodInfo(fileContent, kClassItem, kClassName);
    if (kInfo.has_value()) {
      out.push_back(*kInfo);
    }
  }
}

auto CollectExternMethods(const SL::FileContent* fileContent)
    -> std::vector<ExternMethodInfo> {
  std::vector<ExternMethodInfo> result;

  SL::NodeId const kRoot = fileContent->getRootNode();
  if (!kRoot) {
    return result;
  }

  for (SL::NodeId const kClassDecl : fileContent->sl_collect_all(
           kRoot, SL::VObjectType::paClass_declaration)) {
    CollectExternMethodsFromClassDeclaration(fileContent, kClassDecl, result);
  }

  return result;
}

auto ExtractImplementationKey(const SL::FileContent* fileContent,
                              SL::NodeId bodyDecl) -> std::string {
  SL::NodeId const kClassScope =
      FindChildOfType(fileContent, bodyDecl, SL::VObjectType::paClass_scope);

  if (!kClassScope) {
    return {};
  }

  SL::NodeId const kClassType = fileContent->Child(kClassScope);
  if (!kClassType ||
      fileContent->Type(kClassType) != SL::VObjectType::paClass_type) {
    return {};
  }
  SL::NodeId classNameNode = SL::InvalidNodeId;
  for (SL::NodeId cur = fileContent->Child(kClassType); cur;
       cur = fileContent->Sibling(cur)) {
    if (fileContent->Type(cur) == SL::VObjectType::slStringConst) {
      classNameNode = cur;
    }
  }
  if (!classNameNode) {
    return {};
  }
  std::string_view const kClassName = fileContent->SymName(classNameNode);

  SL::NodeId const kMethodNameNode = fileContent->Sibling(kClassScope);
  if (!kMethodNameNode ||
      fileContent->Type(kMethodNameNode) != SL::VObjectType::slStringConst) {
    return {};
  }
  std::string_view const kMethodName = fileContent->SymName(kMethodNameNode);

  return MakeKey(kClassName, kMethodName);
}

auto CollectImplementedMethods(const SL::FileContent* fileContent)
    -> std::unordered_set<std::string> {
  std::unordered_set<std::string> result;

  SL::NodeId const kRoot = fileContent->getRootNode();
  if (!kRoot) {
    return result;
  }

  for (SL::NodeId const kFuncDecl : fileContent->sl_collect_all(
           kRoot, SL::VObjectType::paFunction_declaration)) {
    SL::NodeId const kBodyDecl = fileContent->Child(kFuncDecl);
    if (!kBodyDecl || fileContent->Type(kBodyDecl) !=
                          SL::VObjectType::paFunction_body_declaration) {
      continue;
    }
    std::string key = ExtractImplementationKey(fileContent, kBodyDecl);
    if (!key.empty()) {
      result.insert(std::move(key));
    }
  }

  for (SL::NodeId const kTaskDecl : fileContent->sl_collect_all(
           kRoot, SL::VObjectType::paTask_declaration)) {
    SL::NodeId const kBodyDecl = fileContent->Child(kTaskDecl);
    if (!kBodyDecl || fileContent->Type(kBodyDecl) !=
                          SL::VObjectType::paTask_body_declaration) {
      continue;
    }
    std::string key = ExtractImplementationKey(fileContent, kBodyDecl);
    if (!key.empty()) {
      result.insert(std::move(key));
    }
  }

  return result;
}

}  // namespace

void CheckMissingFunctionImplementation(SL::Design* design,
                                        SL::ErrorContainer* errors,
                                        SL::SymbolTable* symbols) {
  if (design == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }

  std::vector<ExternMethodInfo> allExternMethods;
  DesignUtils::ForEachFileContent(design, [&](const SL::FileContent* fileCont) {
    auto local = CollectExternMethods(fileCont);
    allExternMethods.insert(allExternMethods.end(), local.begin(), local.end());
  });

  if (allExternMethods.empty()) {
    return;
  }

  std::unordered_set<std::string> implementedMethods;
  for (auto& [name, fileContent] : design->getAllFileContents()) {
    if (fileContent == nullptr) {
      continue;
    }
    auto localImpl = CollectImplementedMethods(fileContent);
    implementedMethods.merge(localImpl);
  }

  for (const auto& externInfo : allExternMethods) {
    std::string const kEy =
        MakeKey(externInfo.className, externInfo.methodName);
    if (implementedMethods.contains(kEy)) {
      continue;
    }

    SL::ErrorDefinition::ErrorType const kErrorType =
        externInfo.isTask ? verihogg_lint::LINT_MISSING_TASK_IMPLEMENTATION
                          : verihogg_lint::LINT_MISSING_FUNCTION_IMPLEMENTATION;

    ReportError(externInfo.fileContent, externInfo.reportNode,
                externInfo.methodName, kErrorType, errors, symbols);
  }
}