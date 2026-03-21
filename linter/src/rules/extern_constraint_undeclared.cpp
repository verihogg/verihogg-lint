#include "rules/extern_constraint_undeclared.h"

#include <cassert>
#include <unordered_map>

#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "utils/ast_utils.h"
#include "utils/location_utils.h"

using namespace SURELOG;

namespace {
std::string getFullNameFromScope(const FileContent* fC, NodeId id) {
  std::stringstream sstream;
  std::string libName{fC->getLibrary()->getName()};
  sstream << libName << "@";

  const NodeId tempId = fC->sl_get(id, VObjectType::paClass_type);
  const std::vector<NodeId> strIds =
      fC->sl_collect_all(tempId, VObjectType::slStringConst);
  assert(strIds.size() > 0);

  std::string firstString{fC->SymName(strIds[0])};
  sstream << firstString;

  for (size_t i = 1; i < strIds.size(); i++) {
    const NodeId stringId = strIds[i];
    std::string scopeName{fC->SymName(stringId)};
    sstream << "::" << scopeName;
  }
  return sstream.str();
}

}  // namespace

void checkExternConstraintUndeclared(const FileContent* fC,
                                     ErrorContainer* errors,
                                     SymbolTable* symbols) {
  if (!fC) return;

  const std::unordered_map<std::string, NodeId> classes = getClassIds(fC);

  const std::vector<NodeId> externConstraintDeclarations = fC->sl_collect_all(
      fC->getRootNode(), VObjectType::paExtern_constraint_declaration);

  for (auto& constrDeclId : externConstraintDeclarations) {
    std::cout << fC->printSubTree(fC->getRootNode()) << std::endl;
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
    reportError(fC, constrDeclId, constrName,
                ErrorDefinition::LINT_EXTERN_CONSTRAINT_UNDECLARED, errors,
                symbols);
  }
}
