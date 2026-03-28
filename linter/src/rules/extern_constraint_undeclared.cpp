#include "rules/extern_constraint_undeclared.h"

#include <cassert>
#include <unordered_map>

#include "main/lint_rules.h"
#include "utils/ast_utils.h"
#include "utils/location_utils.h"

void CheckExternConstraintUndeclared(const SURELOG::FileContent* fileContent,
                                     SURELOG::ErrorContainer* errors,
                                     SURELOG::SymbolTable* symbols) {
  if (fileContent == nullptr) {
    return;
  }

  const std::unordered_map<std::string, SURELOG::NodeId> kClasses =
      GetClassIds(fileContent);

  const std::vector<SURELOG::NodeId> kExternConstraintDeclarations =
      fileContent->sl_collect_all(
          fileContent->getRootNode(),
          SURELOG::VObjectType::paExtern_constraint_declaration);

  for (const auto& constrDeclId : kExternConstraintDeclarations) {
    const SURELOG::NodeId kClassScopeId =
        fileContent->sl_get(constrDeclId, SURELOG::VObjectType::paClass_scope);
    if (kClassScopeId == kZeroId) {
      continue;
    }

    std::string fullName = GetFullNameFromScope(fileContent, kClassScopeId);
    if (!kClasses.contains(fullName)) {
      continue;
    }

    const SURELOG::NodeId kClassId = kClasses.at(fullName);
    const std::string kConstrName = GetStringConst(fileContent, constrDeclId);
    const std::vector<SURELOG::NodeId> kConstrIds = fileContent->sl_collect_all(
        kClassId, SURELOG::VObjectType::paConstraint_prototype);
    bool found = false;
    for (const auto& constrId : kConstrIds) {
      const std::string kProtoName = GetStringConst(fileContent, constrId);
      const SURELOG::NodeId kExternId = fileContent->sl_get(
          constrId, SURELOG::VObjectType::paExtern_qualifier);
      if (kProtoName == kConstrName && kExternId != kZeroId) {
        found = true;
        break;
      }
    }
    if (found) {
      continue;
    }
    ReportError(fileContent, constrDeclId, kConstrName,
                verihogg_lint::LINT_EXTERN_CONSTRAINT_UNDECLARED, errors,
                symbols);
  }
}
