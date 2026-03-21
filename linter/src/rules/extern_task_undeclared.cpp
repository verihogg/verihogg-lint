#include "rules/extern_task_undeclared.h"

#include <cassert>
#include <unordered_map>

#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "utils/ast_utils.h"
#include "utils/location_utils.h"

using namespace SURELOG;

void checkExternTaskUndeclared(const FileContent* fC, ErrorContainer* errors,
                               SymbolTable* symbols) {
  if (!fC) return;
  const std::unordered_map<std::string, NodeId> classes = getClassIds(fC);

  const std::vector<NodeId> taskDeclarations =
      fC->sl_collect_all(fC->getRootNode(), VObjectType::paTask_declaration);

  for (auto& taskId : taskDeclarations) {
    const NodeId parent = fC->Parent(taskId);
    if (fC->Type(parent) == VObjectType::paClass_method) continue;

    const NodeId taskBodyId =
        fC->sl_get(taskId, VObjectType::paTask_body_declaration);

    const NodeId classScopeId =
        fC->sl_get(taskBodyId, VObjectType::paClass_scope);
    if (classScopeId == zeroId) continue;

    std::string fullName = getFullNameFromScope(fC, classScopeId);
    if (classes.find(fullName) == classes.end()) continue;

    const NodeId classId = classes.at(fullName);
    const std::string funcName = getStringConst(fC, taskBodyId);
    const std::vector<NodeId> methodIds =
        fC->sl_collect_all(classId, VObjectType::paClass_method);
    bool found = false;
    for (auto& methodId : methodIds) {
      const NodeId externId =
          fC->sl_collect(methodId, VObjectType::paExtern_qualifier);
      const NodeId protoId =
          fC->sl_collect(methodId, VObjectType::paTask_prototype);
      const std::string protoName = getStringConst(fC, protoId);
      if (protoName == funcName && externId != zeroId) {
        found = true;
        break;
      }
    }
    if (found) continue;
    reportError(fC, taskId, funcName,
                ErrorDefinition::LINT_EXTERN_TASK_UNDECLARED, errors, symbols);
  }
}
