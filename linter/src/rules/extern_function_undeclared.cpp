#include "rules/extern_function_undeclared.h"

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

void CheckExternFunctionUndeclared(SURELOG::Design* design,
                                   SURELOG::ErrorContainer* errors,
                                   SURELOG::SymbolTable* symbols) {
  if (design == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }

  const std::unordered_map<std::string, SURELOG::NodeId> kClasses =
      GetClassIds(design);

  DesignUtils::ForEachFileContent(
      design, [&](const SL::FileContent* fileContent) {
        const std::vector<SURELOG::NodeId> kFuncBodyDeclarations =
            fileContent->sl_collect_all(
                fileContent->getRootNode(),
                SURELOG::VObjectType::paFunction_body_declaration);

        for (const auto& funcId : kFuncBodyDeclarations) {
          const SURELOG::NodeId kClassScopeId =
              fileContent->sl_get(funcId, SURELOG::VObjectType::paClass_scope);
          if (!kClassScopeId) {
            continue;
          }

          const std::string fullName =
              GetFullNameFromScope(fileContent, kClassScopeId);
          if (!kClasses.contains(fullName)) {
            continue;
          }

          const SURELOG::NodeId kClassId = kClasses.at(fullName);
          const std::string kFuncName = GetStringConst(fileContent, funcId);
          const std::vector<SURELOG::NodeId> kMethodIds =
              fileContent->sl_collect_all(kClassId,
                                          SURELOG::VObjectType::paClass_method);
          bool found = false;
          for (const auto& methodId : kMethodIds) {
            const SURELOG::NodeId kExternId = fileContent->sl_collect(
                methodId, SURELOG::VObjectType::paExtern_qualifier);
            const SURELOG::NodeId kProtoId = fileContent->sl_collect(
                methodId, SURELOG::VObjectType::paFunction_prototype);
            const std::string kProtoName =
                GetStringConst(fileContent, kProtoId);
            if (kProtoName == kFuncName && kExternId) {
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
      });
}
