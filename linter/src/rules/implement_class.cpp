#include "rules/implement_class.h"

#include <cassert>
#include <string>
#include <unordered_set>
#include <vector>

#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "utils/ast_utils.h"
#include "utils/location_utils.h"

using namespace SURELOG;

namespace {

std::string getSuperclassStringFromInterfaceClass(const FileContent* fC,
                                                  NodeId id) {
  NodeId classType = id;
  classType = fC->sl_get(classType, VObjectType::paInterface_class_type);
  classType = fC->sl_get(classType, VObjectType::paPs_identifier);
  if (classType == zeroId) return "";
  return getStringConst(fC, classType);
}

}  // namespace

void checkImplementClass(const FileContent* fC, ErrorContainer* errors,
                         SymbolTable* symbols) {
  if (!fC) return;

  std::unordered_set<std::string> interfaceClassSet = getInterfaceClassSet(fC);
  std::unordered_set<std::string> classSet = getClassSet(fC);

  const std::vector<NodeId> classDeclarations =
      fC->sl_collect_all(fC->getRootNode(), VObjectType::paClass_declaration);

  for (auto& classId : classDeclarations) {
    const NodeId implementsId = fC->sl_get(classId, VObjectType::paIMPLEMENTS);
    if (implementsId == zeroId) continue;

    const std::string superclassName =
        getSuperclassStringFromInterfaceClass(fC, classId);

    if (superclassName == "") continue;

    if (classSet.count(superclassName) > 0) {
      std::string className = getStringConst(fC, classId);
      ReportError(fC, classId, className, ErrorDefinition::LINT_IMPLEMENT_CLASS,
                  errors, symbols);
    }
  }
}
