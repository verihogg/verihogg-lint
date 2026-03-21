#include "rules/extern_constraint_undeclared.h"

#include <cassert>
#include <unordered_map>

#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "utils/ast_utils.h"
#include "utils/location_utils.h"

using namespace SURELOG;

void checkExternConstraintUndeclared(const FileContent* fC,
                                     ErrorContainer* errors,
                                     SymbolTable* symbols) {
  if (!fC) return;

  const std::unordered_map<std::string, NodeId> classes = getClassIds(fC);

  const std::vector<NodeId> externConstraintDeclarations = fC->sl_collect_all(
      fC->getRootNode(), VObjectType::paExtern_constraint_declaration);

  for (auto& constrDeclId : externConstraintDeclarations) {
    const NodeId classScopeId =
        fC->sl_get(constrDeclId, VObjectType::paClass_scope);
    if (classScopeId == zeroId) continue;

    std::string fullName = getFullNameFromScope(fC, classScopeId);
    if (classes.find(fullName) == classes.end()) continue;

    const NodeId classId = classes.at(fullName);
    const std::string constrName = getStringConst(fC, constrDeclId);
    const std::vector<NodeId> constrIds =
        fC->sl_collect_all(classId, VObjectType::paConstraint_prototype);
    bool found = false;
    for (auto& constrId : constrIds) {
      const std::string protoName = getStringConst(fC, constrId);
      const NodeId externId =
          fC->sl_get(constrId, VObjectType::paExtern_qualifier);
      if (protoName == constrName && externId != zeroId) {
        found = true;
        break;
      }
    }
    if (found) continue;
    ReportError(fC, constrDeclId, constrName,
                ErrorDefinition::LINT_EXTERN_CONSTRAINT_UNDECLARED, errors,
                symbols);
  }
}
