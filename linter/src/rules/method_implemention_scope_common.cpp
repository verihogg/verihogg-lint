#include "rules/method_implemention_scope_common.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/Design.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/ErrorReporting/ErrorDefinition.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include "main/lint_rules.h"
#include "utils/class_scope_utils.h"
#include "utils/design_utils.h"
#include "utils/location_utils.h"

namespace SL = SURELOG;

namespace {

struct ImplInfo {
  std::string className;
  std::string memberName;
  SL::NodeId reportNode{};
  ClassScopeUtils::ClassScopeInfo scopeInfo{};
  const SL::FileContent* fileContent{};
  ImplKind kind{};
};

using ClassScopeList = std::vector<ClassScopeUtils::ClassScopeInfo>;
using ClassScopeMap = std::unordered_map<std::string, ClassScopeList>;

auto ToOwnedString(std::string_view sv) -> std::string {
  return std::string(sv);
}

auto MakeImplInfo(const SL::FileContent* fileContent, SL::NodeId bodyDecl,
                  ImplKind kind) -> std::optional<ImplInfo> {
  auto const memberInfo =
      ClassScopeUtils::ExtractClassScopedMember(fileContent, bodyDecl);
  if (!memberInfo.has_value()) {
    return std::nullopt;
  }

  SL::NodeId const scopeContainer =
      ClassScopeUtils::FindScopeContainer(fileContent, bodyDecl);
  if (!scopeContainer) {
    return std::nullopt;
  }

  return ImplInfo{
      .className = ToOwnedString(memberInfo->className),
      .memberName = ToOwnedString(memberInfo->memberName),
      .reportNode = memberInfo->classScopeNode,
      .scopeInfo =
          ClassScopeUtils::ClassScopeInfo{
              .scopeType = fileContent->Type(scopeContainer),
              .scopeName = ClassScopeUtils::GetScopeContainerName(
                  fileContent, scopeContainer),
          },
      .fileContent = fileContent,
      .kind = kind,
  };
}

auto CollectImplInfos(const SL::FileContent* fileContent)
    -> std::vector<ImplInfo> {
  std::vector<ImplInfo> result;

  if (fileContent == nullptr) {
    return result;
  }

  SL::NodeId const root = fileContent->getRootNode();
  if (!root) {
    return result;
  }

  for (SL::NodeId const funcDecl : fileContent->sl_collect_all(
           root, SL::VObjectType::paFunction_declaration)) {
    SL::NodeId const body = fileContent->Child(funcDecl);
    if (body && fileContent->Type(body) ==
                    SL::VObjectType::paFunction_body_declaration) {
      if (auto info = MakeImplInfo(fileContent, body, ImplKind::kFunction)) {
        result.push_back(std::move(*info));
      }
    }
  }

  for (SL::NodeId const taskDecl :
       fileContent->sl_collect_all(root, SL::VObjectType::paTask_declaration)) {
    SL::NodeId const body = fileContent->Child(taskDecl);
    if (body &&
        fileContent->Type(body) == SL::VObjectType::paTask_body_declaration) {
      if (auto info = MakeImplInfo(fileContent, body, ImplKind::kTask)) {
        result.push_back(std::move(*info));
      }
    }
  }

  for (SL::NodeId const constraintNode : fileContent->sl_collect_all(
           root, SL::VObjectType::paExtern_constraint_declaration)) {
    if (auto info =
            MakeImplInfo(fileContent, constraintNode, ImplKind::kConstraint)) {
      result.push_back(std::move(*info));
    }
  }

  return result;
}

auto CollectClassScopeMap(SL::Design* design) -> ClassScopeMap {
  ClassScopeMap result;

  if (design == nullptr) {
    return result;
  }

  DesignUtils::ForEachFileContent(design, [&](const SL::FileContent* fileCont) {
    if (fileCont == nullptr) {
      return;
    }

    for (const auto& classDeclInfo :
         ClassScopeUtils::CollectClassDeclInfos(fileCont)) {
      result[ToOwnedString(classDeclInfo.className)].push_back(
          classDeclInfo.scopeInfo);
    }
  });

  return result;
}

auto HasMatchingClassScope(const ClassScopeMap& classScopeMap,
                           const ImplInfo& impl) -> bool {
  auto const iter = classScopeMap.find(impl.className);
  if (iter == classScopeMap.end()) {
    return false;
  }

  return std::any_of(iter->second.begin(), iter->second.end(),
                     [&](const ClassScopeUtils::ClassScopeInfo& classScope) {
                       return ClassScopeUtils::ScopesMatch(impl.scopeInfo,
                                                           classScope);
                     });
}

auto ErrorTypeFor(ImplKind kind) -> verihogg_lint::LintId {
  switch (kind) {
    case ImplKind::kFunction:
      return verihogg_lint::LINT_FUNC_IMPL_SCOPE;
    case ImplKind::kTask:
      return verihogg_lint::LINT_TASK_IMPL_SCOPE;
    case ImplKind::kConstraint:
      return verihogg_lint::LINT_CONSTRAINT_IMPL_SCOPE;
  }

  return verihogg_lint::LINT_FUNC_IMPL_SCOPE;
}

}  // namespace

void CheckImplScopeForKind(SL::Design* design, SL::ErrorContainer* errors,
                           SL::SymbolTable* symbols, ImplKind kind) {
  if (design == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }

  const ClassScopeMap classScopeMap = CollectClassScopeMap(design);
  if (classScopeMap.empty()) {
    return;
  }

  std::vector<ImplInfo> impls;
  DesignUtils::ForEachFileContent(design, [&](const SL::FileContent* fileCont) {
    for (auto& info : CollectImplInfos(fileCont)) {
      if (info.kind == kind) {
        impls.push_back(std::move(info));
      }
    }
  });

  for (const auto& impl : impls) {
    if (impl.fileContent == nullptr || !impl.reportNode) {
      continue;
    }
    if (HasMatchingClassScope(classScopeMap, impl)) {
      continue;
    }

    ReportError(impl.fileContent, impl.reportNode, impl.memberName,
                ErrorTypeFor(impl.kind), errors, symbols);
  }
}
