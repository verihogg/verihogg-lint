#include "rules/extern_function_undeclared.h"

#include <cassert>
#include <sstream>
#include <unordered_map>
#include <vector>

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

    const NodeId classId = classes.at(fullName);
    const std::string funcName = getStringConst(fC, funcId);
    const std::vector<NodeId> methodIds =
        fC->sl_collect_all(classId, VObjectType::paClass_method);
    bool found = false;
    for (auto& methodId : methodIds) {
      const NodeId externId =
          fC->sl_collect(methodId, VObjectType::paExtern_qualifier);
      const NodeId protoId =
          fC->sl_collect(methodId, VObjectType::paFunction_prototype);
      const std::string protoName = getStringConst(fC, protoId);
      if (protoName == funcName && externId != zeroId) {
        found = true;
        break;
      }
    }
    if (found) continue;
    ReportError(fC, funcId, funcName,
                ErrorDefinition::LINT_EXTERN_FUNCTION_UNDECLARED, errors,
                symbols);
  }
}
