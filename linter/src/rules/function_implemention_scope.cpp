#include "rules/function_implemention_scope.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/Design.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

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

struct ImplInfo {
  std::string_view className;
  std::string_view methodName;
  SL::NodeId reportNode;
  ClassScopeUtils::ClassScopeInfo scopeInfo;
  const SL::FileContent* fileContent;
};

auto ExtractImplInfo(const SL::FileContent* fileContent, SL::NodeId bodyDecl)
    -> std::optional<ImplInfo> {
  SL::NodeId const classScope =
      FindChildOfType(fileContent, bodyDecl, SL::VObjectType::paClass_scope);
  if (!classScope) {
    return std::nullopt;
  }

  SL::NodeId const classType = fileContent->Child(classScope);
  if (!classType ||
      fileContent->Type(classType) != SL::VObjectType::paClass_type) {
    return std::nullopt;
  }

  SL::NodeId classNameNode = SL::InvalidNodeId;
  for (SL::NodeId cur = fileContent->Child(classType); cur;
       cur = fileContent->Sibling(cur)) {
    if (fileContent->Type(cur) == SL::VObjectType::slStringConst) {
      classNameNode = cur;
    }
  }
  if (!classNameNode) {
    return std::nullopt;
  }
  std::string_view const className = fileContent->SymName(classNameNode);

  SL::NodeId const methodNameNode = fileContent->Sibling(classScope);
  if (!methodNameNode ||
      fileContent->Type(methodNameNode) != SL::VObjectType::slStringConst) {
    return std::nullopt;
  }
  std::string_view const methodName = fileContent->SymName(methodNameNode);

  SL::NodeId const scopeContainer =
      ClassScopeUtils::FindScopeContainer(fileContent, bodyDecl);
  if (!scopeContainer) {
    return std::nullopt;
  }

  return ImplInfo{
      .className = className,
      .methodName = methodName,
      .reportNode = classScope,
      .scopeInfo =
          ClassScopeUtils::ClassScopeInfo{
              .scopeType = fileContent->Type(scopeContainer),
              .scopeName = ClassScopeUtils::GetScopeContainerName(
                  fileContent, scopeContainer),
          },
      .fileContent = fileContent,
  };
}

auto CollectImplInfos(const SL::FileContent* fileContent)
    -> std::vector<ImplInfo> {
  std::vector<ImplInfo> result;

  SL::NodeId const root = fileContent->getRootNode();
  if (!root) {
    return result;
  }

  for (SL::NodeId const funcDecl : fileContent->sl_collect_all(
           root, SL::VObjectType::paFunction_declaration)) {
    SL::NodeId const bodyDecl = fileContent->Child(funcDecl);
    if (!bodyDecl || fileContent->Type(bodyDecl) !=
                         SL::VObjectType::paFunction_body_declaration) {
      continue;
    }
    auto info = ExtractImplInfo(fileContent, bodyDecl);
    if (info.has_value()) {
      result.push_back(std::move(*info));
    }
  }

  for (SL::NodeId const taskDecl :
       fileContent->sl_collect_all(root, SL::VObjectType::paTask_declaration)) {
    SL::NodeId const bodyDecl = fileContent->Child(taskDecl);
    if (!bodyDecl || fileContent->Type(bodyDecl) !=
                         SL::VObjectType::paTask_body_declaration) {
      continue;
    }
    auto info = ExtractImplInfo(fileContent, bodyDecl);
    if (info.has_value()) {
      result.push_back(std::move(*info));
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
  DesignUtils::ForEachFileContent(design, [&](const SL::FileContent* fc) {
    globalClassScopeMap.merge(ClassScopeUtils::BuildClassScopeMap(fc));
  });

  if (globalClassScopeMap.empty()) {
    return;
  }

  std::vector<ImplInfo> allImpls;
  DesignUtils::ForEachFileContent(design, [&](const SL::FileContent* fc) {
    auto local = CollectImplInfos(fc);
    allImpls.insert(allImpls.end(), local.begin(), local.end());
  });

  for (const auto& impl : allImpls) {
    auto const it = globalClassScopeMap.find(impl.className);
    if (it == globalClassScopeMap.end()) {
      continue;
    }

    if (ClassScopeUtils::ScopesMatch(impl.scopeInfo, it->second)) {
      continue;
    }

    ReportError(impl.fileContent, impl.reportNode, impl.methodName,
                verihogg_lint::LINT_FUNC_IMPL_SCOPE, errors, symbols);
  }
}