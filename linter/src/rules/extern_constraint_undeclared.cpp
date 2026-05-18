#include "rules/extern_constraint_undeclared.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/Design.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <cassert>
#include <string>
#include <unordered_map>
#include <vector>

#include "main/lint_rules.h"
#include "utils/ast_utils.h"
#include "utils/design_utils.h"
#include "utils/location_utils.h"

void CheckExternConstraintUndeclared(SURELOG::Design* design,
                                     SURELOG::ErrorContainer* errors,
                                     SURELOG::SymbolTable* symbols) {
  if (design == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }

  const std::unordered_map<std::string, SURELOG::NodeId> kClasses =
      GetClassIds(design);

  DesignUtils::ForEachFileContent(
      design, [&](const SL::FileContent* fileContent) {
        const std::vector<SURELOG::NodeId> kExternConstraintDeclarations =
            fileContent->sl_collect_all(
                fileContent->getRootNode(),
                SURELOG::VObjectType::paExtern_constraint_declaration);

        for (const auto& constrDeclId : kExternConstraintDeclarations) {
          const SURELOG::NodeId kClassScopeId = fileContent->sl_get(
              constrDeclId, SURELOG::VObjectType::paClass_scope);
          if (!kClassScopeId) {
            continue;
          }

          const std::string fullName =
              GetFullNameFromScope(fileContent, kClassScopeId);
          if (!kClasses.contains(fullName)) {
            continue;
          }

          const SURELOG::NodeId kClassId = kClasses.at(fullName);
          const std::string kConstrName =
              GetStringConst(fileContent, constrDeclId);
          const std::vector<SURELOG::NodeId> kConstrIds =
              fileContent->sl_collect_all(
                  kClassId, SURELOG::VObjectType::paConstraint_prototype);
          bool found = false;
          for (const auto& constrId : kConstrIds) {
            const std::string kProtoName =
                GetStringConst(fileContent, constrId);
            const SURELOG::NodeId kExternId = fileContent->sl_get(
                constrId, SURELOG::VObjectType::paExtern_qualifier);
            if (kProtoName == kConstrName && kExternId) {
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
      });
}
