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

  const std::vector<NodeId> constrDeclarations = fC->sl_collect_all(
      fC->getRootNode(), VObjectType::paExtern_constraint_declaration);

  for (auto& constrId : constrDeclarations) {
    NodeId stringId = fC->sl_get(constrId, VObjectType::paClass_scope);
    stringId = fC->sl_collect(stringId, VObjectType::slStringConst);

    const std::string constrName = getStringConst(fC, constrId);
    const std::string className =
        getPrefix(fC, constrId) + getStringConst(fC, stringId);

    assert(classes.find(className) != classes.end());
    const NodeId classId = classes.at(className);

    const std::vector<NodeId> constrPrototypes =
        fC->sl_collect_all(classId, VObjectType::paConstraint_prototype);

    for (auto& protoId : constrPrototypes) {
      const NodeId nameId = fC->sl_collect(protoId, VObjectType::slStringConst);
      const NodeId externId =
          fC->sl_collect(protoId, VObjectType::paExtern_qualifier);
      const std::string protoName = getStringConst(fC, nameId);
      if (protoName == constrName && externId == zeroId) {
        ReportError(fC, protoId, protoName,
                    ErrorDefinition::LINT_EXTERN_CONSTRAINT_UNDECLARED, errors,
                    symbols);
      }
    }
  }
}
