#include "rules/extern_function_undeclared.h"

#include <cassert>
#include <sstream>
#include <unordered_map>
#include <vector>

#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "utils/ast_utils.h"
#include "utils/location_utils.h"

void checkExternFunctionUndeclared(const SURELOG::FileContent* fC,
                                   SURELOG::ErrorContainer* errors,
                                   SURELOG::SymbolTable* symbols) {
  if (fC == nullptr) {
    return;
  }

  const std::unordered_map<std::string, SURELOG::NodeId> kClasses =
      getClassIds(fC);

  const std::vector<SURELOG::NodeId> kFuncBodyDeclarations = fC->sl_collect_all(
      fC->getRootNode(), VObjectType::paFunction_body_declaration);

  for (auto& funcId : kFuncBodyDeclarations) {
    const SURELOG::NodeId classScopeId =
        fC->sl_get(funcId, VObjectType::paClass_scope);
    if (classScopeId == kZeroId) {
      continue;
    }

    std::string fullName = getFullNameFromScope(fC, classScopeId);
    if (kClasses.find(fullName) == kClasses.end()) {
      continue;
    }

    const SURELOG::NodeId classId = kClasses.at(fullName);
    const std::string funcName = getStringConst(fC, funcId);
    const std::vector<SURELOG::NodeId> methodIds =
        fC->sl_collect_all(classId, VObjectType::paClass_method);
    bool found = false;
    for (auto& methodId : methodIds) {
      const SURELOG::NodeId externId =
          fC->sl_collect(methodId, VObjectType::paExtern_qualifier);
      const SURELOG::NodeId protoId =
          fC->sl_collect(methodId, VObjectType::paFunction_prototype);
      const std::string protoName = getStringConst(fC, protoId);
      if (protoName == funcName && externId != kZeroId) {
        found = true;
        break;
      }
    }
    if (found) {
      continue;
    }
    ReportError(fC, funcId, funcName,
                ErrorDefinition::LINT_EXTERN_FUNCTION_UNDECLARED, errors,
                symbols);
  }
}
