#include "rules/function_implemention_scope.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/Design.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/ErrorReporting/ErrorDefinition.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <cstdint>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include "main/lint_rules.h"
#include "utils/ast_utils.h"
#include "utils/class_scope_utils.h"
#include "utils/design_utils.h"
#include "utils/location_utils.h"

namespace SL = SURELOG;

namespace {

enum class ImplKind : std::uint8_t { kFunction, kTask, kConstraint };

struct ImplInfo {
  std::string_view className;
  std::string_view memberName;
  SL::NodeId reportNode;
  ClassScopeUtils::ClassScopeInfo scopeInfo;
  const SL::FileContent* fileContent;
  ImplKind kind;
};

auto ExtractSubroutineImplInfo(const SL::FileContent* fileContent,
                               SL::NodeId bodyDecl, ImplKind kind)
    -> std::optional<ImplInfo> {
  SL::NodeId const kClassScope =
      FindChildOfType(fileContent, bodyDecl, SL::VObjectType::paClass_scope);
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
  std::string_view const kClassName = fileContent->SymName(classNameNode);

  SL::NodeId const kMethodNameNode = fileContent->Sibling(kClassScope);
  if (!kMethodNameNode ||
      fileContent->Type(kMethodNameNode) != SL::VObjectType::slStringConst) {
    return std::nullopt;
  }
  std::string_view const kMemberName = fileContent->SymName(kMethodNameNode);

  SL::NodeId const kScopeContainer =
      ClassScopeUtils::FindScopeContainer(fileContent, bodyDecl);
  if (!kScopeContainer) {
    return std::nullopt;
  }

  return ImplInfo{.className = kClassName,
                  .memberName = kMemberName,
                  .reportNode = kClassScope,
                  .scopeInfo =
                      ClassScopeUtils::ClassScopeInfo{
                          .scopeType = fileContent->Type(kScopeContainer),
                          .scopeName = ClassScopeUtils::GetScopeContainerName(
                              fileContent, kScopeContainer),
                      },
                  .fileContent = fileContent,
                  .kind = kind};
}

auto ExtractConstraintImplInfo(const SL::FileContent* fileContent,
                               SL::NodeId node) -> std::optional<ImplInfo> {
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

  std::string_view const kClassName = fileContent->SymName(classNameNode);

  SL::NodeId const kConstraintNameNode = fileContent->Sibling(kClassScope);
  if (!kConstraintNameNode || fileContent->Type(kConstraintNameNode) !=
                                  SL::VObjectType::slStringConst) {
    return std::nullopt;
  }

  std::string_view const kConstraintName =
      fileContent->SymName(kConstraintNameNode);

  SL::NodeId const kScopeContainer =
      ClassScopeUtils::FindScopeContainer(fileContent, node);
  if (!kScopeContainer) {
    return std::nullopt;
  }

  return ImplInfo{.className = kClassName,
                  .memberName = kConstraintName,
                  .reportNode = kClassScope,
                  .scopeInfo =
                      ClassScopeUtils::ClassScopeInfo{
                          .scopeType = fileContent->Type(kScopeContainer),
                          .scopeName = ClassScopeUtils::GetScopeContainerName(
                              fileContent, kScopeContainer)},
                  .fileContent = fileContent,
                  .kind = ImplKind::kConstraint};
}

auto CollectImplInfos(const SL::FileContent* fileContent)
    -> std::vector<ImplInfo> {
  std::vector<ImplInfo> result;

  SL::NodeId const kRoot = fileContent->getRootNode();
  if (!kRoot) {
    return result;
  }

  for (SL::NodeId const kFuncDecl : fileContent->sl_collect_all(
           kRoot, SL::VObjectType::paFunction_declaration)) {
    SL::NodeId const kBody = fileContent->Child(kFuncDecl);
    if (kBody && fileContent->Type(kBody) ==
                     SL::VObjectType::paFunction_body_declaration) {
      auto info =
          ExtractSubroutineImplInfo(fileContent, kBody, ImplKind::kFunction);
      if (info) {
        result.push_back(*info);
      }
    }
  }

  for (SL::NodeId const kTaskDecl : fileContent->sl_collect_all(
           kRoot, SL::VObjectType::paTask_declaration)) {
    SL::NodeId const kBody = fileContent->Child(kTaskDecl);
    if (kBody &&
        fileContent->Type(kBody) == SL::VObjectType::paTask_body_declaration) {
      auto info =
          ExtractSubroutineImplInfo(fileContent, kBody, ImplKind::kTask);
      if (info) {
        result.push_back(*info);
      }
    }
  }

  for (SL::NodeId const kNode : fileContent->sl_collect_all(
           kRoot, SL::VObjectType::paExtern_constraint_declaration)) {
    auto info = ExtractConstraintImplInfo(fileContent, kNode);
    if (info) {
      result.push_back(*info);
    }
  }

  return result;
}

}  // namespace

void CheckFuncImplScope(SL::Design* design, SL::ErrorContainer* errors,
                        SL::SymbolTable* symbols) {
  if (design == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }

  std::unordered_map<std::string_view, ClassScopeUtils::ClassScopeInfo>
      globalClassScopeMap;
  DesignUtils::ForEachFileContent(design, [&](const SL::FileContent* fileCont) {
    globalClassScopeMap.merge(ClassScopeUtils::BuildClassScopeMap(fileCont));
  });

  if (globalClassScopeMap.empty()) {
    return;
  }

  std::vector<ImplInfo> allImpls;
  DesignUtils::ForEachFileContent(design, [&](const SL::FileContent* fileCont) {
    auto local = CollectImplInfos(fileCont);
    allImpls.insert(allImpls.end(), local.begin(), local.end());
  });

  for (const auto& impl : allImpls) {
    auto iter = globalClassScopeMap.find(impl.className);
    if (iter == globalClassScopeMap.end()) {
      continue;
    }

    if (ClassScopeUtils::ScopesMatch(impl.scopeInfo, iter->second)) {
      continue;
    }

    SL::ErrorDefinition::ErrorType errorType{};

    switch (impl.kind) {
      case ImplKind::kFunction:
        errorType = verihogg_lint::LINT_FUNC_IMPL_SCOPE;
        break;

      case ImplKind::kTask:
        errorType = verihogg_lint::LINT_TASK_IMPL_SCOPE;
        break;

      case ImplKind::kConstraint:
        errorType = verihogg_lint::LINT_CONSTRAINT_IMPL_SCOPE;
        break;
    }

    ReportError(impl.fileContent, impl.reportNode, impl.memberName, errorType,
                errors, symbols);
  }
}