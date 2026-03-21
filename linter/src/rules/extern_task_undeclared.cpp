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

  const std::vector<NodeId> taskBodyDeclarations = fC->sl_collect_all(
      fC->getRootNode(), VObjectType::paTask_body_declaration);

  for (auto& taskBodyId : taskBodyDeclarations) {
    const std::string declName = getStringConst(fC, taskBodyId);
    const std::string className = getPrefix(fC, taskBodyId);

    if (isBuiltinClass(removeFilePrefix(className))) continue;
    if (classes.count(className) == 0) continue;
    const NodeId classId = classes.at(className);

    const std::vector<NodeId> taskPrototypes =
        fC->sl_collect_all(classId, VObjectType::paTask_declaration);
    for (auto& protoId : taskPrototypes) {
      const NodeId nameId = fC->sl_collect(protoId, VObjectType::slStringConst);
      const NodeId externId =
          fC->sl_collect(protoId, VObjectType::paExtern_qualifier);
      const std::string protoName = getStringConst(fC, nameId);

      if (protoName == declName && externId == zeroId) {
        ReportError(fC, classId, className,
                    ErrorDefinition::LINT_EXTERN_TASK_UNDECLARED, errors,
                    symbols);
      }
    }
  }
}
