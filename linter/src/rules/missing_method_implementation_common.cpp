#include "rules/missing_method_implementation_common.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/Design.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/ErrorReporting/ErrorDefinition.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

#include "main/lint_rules.h"
#include "utils/ast_utils.h"
#include "utils/class_scope_utils.h"
#include "utils/design_utils.h"
#include "utils/location_utils.h"

namespace SL = SURELOG;

namespace {

auto KindTag(MethodKind kind) -> std::string_view {
  switch (kind) {
    case MethodKind::kFunction:
      return "#function";
    case MethodKind::kTask:
      return "#task";
  }
  return "#unknown";
}

auto MakeKey(std::string_view className, std::string_view methodName,
             MethodKind kind) -> std::string {
  std::string key;
  key.reserve(className.size() + 2 + methodName.size() + KindTag(kind).size());
  key.append(className);
  key.append("::");
  key.append(methodName);
  key.append(KindTag(kind));
  return key;
}

auto ExtractMethodNameFromPrototype(const SL::FileContent* fileContent,
                                    SL::NodeId methodProto)
    -> std::string_view {
  SL::NodeId const firstChild = fileContent->Child(methodProto);
  if (!firstChild) {
    return {};
  }

  SL::VObjectType const protoType = fileContent->Type(methodProto);

  if (protoType == SL::VObjectType::paFunction_prototype) {
    if (fileContent->Type(firstChild) !=
        SL::VObjectType::paFunction_data_type_or_implicit) {
      return {};
    }

    SL::NodeId const nameNode = fileContent->Sibling(firstChild);
    if (!nameNode ||
        fileContent->Type(nameNode) != SL::VObjectType::slStringConst) {
      return {};
    }

    return fileContent->SymName(nameNode);
  }

  if (protoType == SL::VObjectType::paTask_prototype) {
    if (fileContent->Type(firstChild) != SL::VObjectType::slStringConst) {
      return {};
    }

    return fileContent->SymName(firstChild);
  }

  return {};
}

struct ExternMethodInfo {
  std::string_view className;
  std::string_view methodName;
  SL::NodeId reportNode;
  const SL::FileContent* fileContent;
  MethodKind kind;
};

auto GetClassNameFromClassDeclaration(const SL::FileContent* fileContent,
                                      SL::NodeId classDecl)
    -> std::string_view {
  SL::NodeId const classKw = fileContent->Child(classDecl);
  if (!classKw || fileContent->Type(classKw) != SL::VObjectType::paCLASS) {
    return {};
  }

  SL::NodeId const classNameNode = fileContent->Sibling(classKw);
  if (!classNameNode ||
      fileContent->Type(classNameNode) != SL::VObjectType::slStringConst) {
    return {};
  }

  return fileContent->SymName(classNameNode);
}

auto TryCollectExternMethodInfo(const SL::FileContent* fileContent,
                                SL::NodeId classItem,
                                std::string_view className)
    -> std::optional<ExternMethodInfo> {
  SL::NodeId const classMethod = fileContent->Child(classItem);
  if (!classMethod ||
      fileContent->Type(classMethod) != SL::VObjectType::paClass_method) {
    return std::nullopt;
  }

  SL::NodeId const firstChild = fileContent->Child(classMethod);
  if (!firstChild ||
      fileContent->Type(firstChild) != SL::VObjectType::paExtern_qualifier) {
    return std::nullopt;
  }

  SL::NodeId const methodProtoContainer = FindSiblingOfType(
      fileContent, firstChild, SL::VObjectType::paMethod_prototype);
  if (!methodProtoContainer) {
    return std::nullopt;
  }

  SL::NodeId const proto = fileContent->Child(methodProtoContainer);
  if (!proto) {
    return std::nullopt;
  }

  SL::VObjectType const protoType = fileContent->Type(proto);
  MethodKind kind{};
  if (protoType == SL::VObjectType::paFunction_prototype) {
    kind = MethodKind::kFunction;
  } else if (protoType == SL::VObjectType::paTask_prototype) {
    kind = MethodKind::kTask;
  } else {
    return std::nullopt;
  }

  std::string_view const methodName =
      ExtractMethodNameFromPrototype(fileContent, proto);
  if (methodName.empty()) {
    return std::nullopt;
  }

  return ExternMethodInfo{
      .className = className,
      .methodName = methodName,
      .reportNode = methodProtoContainer,
      .fileContent = fileContent,
      .kind = kind,
  };
}

auto CollectExternMethodsFromClassDeclaration(
    const SL::FileContent* fileContent, SL::NodeId classDecl,
    std::vector<ExternMethodInfo>& out) -> void {
  std::string_view const className =
      GetClassNameFromClassDeclaration(fileContent, classDecl);
  if (className.empty()) {
    return;
  }

  for (SL::NodeId const classItem :
       fileContent->sl_collect_all(classDecl, SL::VObjectType::paClass_item)) {
    std::optional<ExternMethodInfo> const info =
        TryCollectExternMethodInfo(fileContent, classItem, className);
    if (info.has_value()) {
      out.push_back(*info);
    }
  }
}

auto CollectExternMethods(const SL::FileContent* fileContent)
    -> std::vector<ExternMethodInfo> {
  std::vector<ExternMethodInfo> result;

  SL::NodeId const root = fileContent->getRootNode();
  if (!root) {
    return result;
  }

  for (SL::NodeId const classDecl : fileContent->sl_collect_all(
           root, SL::VObjectType::paClass_declaration)) {
    CollectExternMethodsFromClassDeclaration(fileContent, classDecl, result);
  }

  return result;
}

auto ExtractImplementationKey(const SL::FileContent* fileContent,
                              SL::NodeId bodyDecl, MethodKind kind)
    -> std::string {
  auto const memberInfo =
      ClassScopeUtils::ExtractClassScopedMember(fileContent, bodyDecl);
  if (!memberInfo.has_value()) {
    return {};
  }

  return MakeKey(memberInfo->className, memberInfo->memberName, kind);
}

auto CollectImplementedMethodsOfKind(const SL::FileContent* fileContent,
                                     MethodKind kind)
    -> std::unordered_set<std::string> {
  std::unordered_set<std::string> result;

  SL::NodeId const root = fileContent->getRootNode();
  if (!root) {
    return result;
  }

  if (kind == MethodKind::kFunction) {
    for (SL::NodeId const funcDecl : fileContent->sl_collect_all(
             root, SL::VObjectType::paFunction_declaration)) {
      SL::NodeId const bodyDecl = fileContent->Child(funcDecl);
      if (!bodyDecl || fileContent->Type(bodyDecl) !=
                           SL::VObjectType::paFunction_body_declaration) {
        continue;
      }

      std::string key = ExtractImplementationKey(fileContent, bodyDecl, kind);
      if (!key.empty()) {
        result.insert(std::move(key));
      }
    }
    return result;
  }

  for (SL::NodeId const taskDecl :
       fileContent->sl_collect_all(root, SL::VObjectType::paTask_declaration)) {
    SL::NodeId const bodyDecl = fileContent->Child(taskDecl);
    if (!bodyDecl || fileContent->Type(bodyDecl) !=
                         SL::VObjectType::paTask_body_declaration) {
      continue;
    }

    std::string key = ExtractImplementationKey(fileContent, bodyDecl, kind);
    if (!key.empty()) {
      result.insert(std::move(key));
    }
  }

  return result;
}

auto CollectImplementedMethods(SL::Design* design, MethodKind kind)
    -> std::unordered_set<std::string> {
  std::unordered_set<std::string> result;

  for (auto& [name, fileContent] : design->getAllFileContents()) {
    if (fileContent == nullptr) {
      continue;
    }

    auto localImpl = CollectImplementedMethodsOfKind(fileContent, kind);
    result.merge(localImpl);
  }

  return result;
}

}  // namespace

void CheckMissingMethodImplementation(SL::Design* design,
                                      SL::ErrorContainer* errors,
                                      SL::SymbolTable* symbols,
                                      MethodKind kind) {
  if (design == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }

  std::vector<ExternMethodInfo> externMethods;
  DesignUtils::ForEachFileContent(design, [&](const SL::FileContent* fileCont) {
    for (auto& info : CollectExternMethods(fileCont)) {
      if (info.kind == kind) {
        externMethods.push_back(std::move(info));
      }
    }
  });

  if (externMethods.empty()) {
    return;
  }

  std::unordered_set<std::string> implementedMethods =
      CollectImplementedMethods(design, kind);

  const auto errorType =
      kind == MethodKind::kTask
          ? verihogg_lint::LINT_MISSING_TASK_IMPLEMENTATION
          : verihogg_lint::LINT_MISSING_FUNCTION_IMPLEMENTATION;

  for (const auto& externInfo : externMethods) {
    std::string const key =
        MakeKey(externInfo.className, externInfo.methodName, externInfo.kind);
    if (!implementedMethods.contains(key)) {
      ReportError(externInfo.fileContent, externInfo.reportNode,
                  externInfo.methodName, errorType, errors, symbols);
    }
  }
}
