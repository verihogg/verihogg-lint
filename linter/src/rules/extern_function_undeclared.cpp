#include "rules/extern_function_undeclared.h"

#include <cassert>
#include <unordered_map>
#include <vector>

#include "main/lint_rules.h"
#include "utils/ast_utils.h"
#include "utils/location_utils.h"

void CheckExternFunctionUndeclared(const SURELOG::FileContent* fileContent,
                                   SURELOG::ErrorContainer* errors,
                                   SURELOG::SymbolTable* symbols) {
  if (fileContent == nullptr) {
    return;
  }

  const std::unordered_map<std::string, SURELOG::NodeId> kClasses =
      GetClassIds(fileContent);

  const std::vector<SURELOG::NodeId> kFuncBodyDeclarations =
      fileContent->sl_collect_all(
          fileContent->getRootNode(),
          SURELOG::VObjectType::paFunction_body_declaration);

  for (const auto& funcId : kFuncBodyDeclarations) {
    const SURELOG::NodeId kClassScopeId =
        fileContent->sl_get(funcId, SURELOG::VObjectType::paClass_scope);
    if (kClassScopeId == kZeroId) {
      continue;
    }

    std::string fullName = GetFullNameFromScope(fileContent, kClassScopeId);
    if (!kClasses.contains(fullName)) {
      continue;
    }

    const SURELOG::NodeId kClassId = kClasses.at(fullName);
    const std::string kFuncName = GetStringConst(fileContent, funcId);
    const std::vector<SURELOG::NodeId> kMethodIds = fileContent->sl_collect_all(
        kClassId, SURELOG::VObjectType::paClass_method);
    bool found = false;
    for (const auto& methodId : kMethodIds) {
      const SURELOG::NodeId kExternId = fileContent->sl_collect(
          methodId, SURELOG::VObjectType::paExtern_qualifier);
      const SURELOG::NodeId kProtoId = fileContent->sl_collect(
          methodId, SURELOG::VObjectType::paFunction_prototype);
      const std::string kProtoName = GetStringConst(fileContent, kProtoId);
      if (kProtoName == kFuncName && kExternId != kZeroId) {
        found = true;
        break;
      }
    }
    if (found) {
      continue;
    }
    ReportError(fileContent, funcId, kFuncName,
                verihogg_lint::LINT_EXTERN_FUNCTION_UNDECLARED, errors,
                symbols);
  }
}
