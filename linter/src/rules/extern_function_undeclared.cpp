#include "rules/extern_function_undeclared.h"

#include <cassert>
#include <unordered_map>

#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "utils/ast_utils.h"
#include "utils/location_utils.h"

using namespace SURELOG;

void checkExternFunctionUndeclared(const FileContent* fC,
                                   ErrorContainer* errors,
                                   SymbolTable* symbols) {
  if (!fC) return;

  const std::unordered_map<std::string, NodeId> classes = getClassIds(fC);

  const std::vector<NodeId> funcBodyDeclarations = fC->sl_collect_all(
      fC->getRootNode(), VObjectType::paFunction_body_declaration);

  for (auto& funcBodyId : funcBodyDeclarations) {
    const std::string declName = getStringConst(fC, funcBodyId);
    const std::string className = getPrefix(fC, funcBodyId);

    if (isBuiltinClass(removeFilePrefix(className))) continue;
    if (classes.count(className) == 0) continue;
    const NodeId classId = classes.at(className);

    const std::vector<NodeId> funcPrototypes =
        fC->sl_collect_all(classId, VObjectType::paFunction_body_declaration);

    for (auto& protoId : funcPrototypes) {
      const std::string protoName = getStringConst(fC, protoId);
      const NodeId externId =
          fC->sl_collect(protoId, VObjectType::paExtern_qualifier);

      if (protoName == declName && externId == zeroId) {
        ReportError(fC, classId, className,
                    ErrorDefinition::LINT_EXTERN_FUNCTION_UNDECLARED, errors,
                    symbols);
      }
    }
  }
}
