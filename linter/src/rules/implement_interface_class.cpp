#include "rules/implement_interface_class.h"

#include <cassert>
#include <string>
#include <unordered_set>
#include <vector>

#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "utils/ast_utils.h"
#include "utils/location_utils.h"

namespace {
auto getSuperInterfaceString(const FileContent* fC, SURELOG::NodeId id)
    -> std::string {
  SURELOG::NodeId classType = id;
  classType = fC->sl_get(classType, VObjectType::paInterface_class_type);
  classType = fC->sl_get(classType, VObjectType::paPs_identifier);
  if (classType == zeroId) {
    return "";
  }
  return getStringConst(fC, classType);
}
}  // namespace

void checkImplementInterfaceClass(const FileContent* fC, ErrorContainer* errors,
                                  SymbolTable* symbols) {
  if (!fC) {
    return;
  }

  std::unordered_set<std::string> classSet = getClassSet(fC);
  std::unordered_set<std::string> interfaceClassSet = getInterfaceClassSet(fC);

  const std::vector<SURELOG::NodeId> classDeclarations =
      fC->sl_collect_all(fC->getRootNode(), VObjectType::paClass_declaration);

  for (auto& classId : classDeclarations) {
    const SURELOG::NodeId implementsId =
        fC->sl_get(classId, VObjectType::paIMPLEMENTS);
    if (implementsId == zeroId) {
      continue;
    }

    const std::string superInterfaceName = getSuperInterfaceString(fC, classId);
    if (superInterfaceName == "") {
      continue;
    }

    if (interfaceClassSet.count(superInterfaceName) == 0) {
      std::string className = getStringConst(fC, classId);
      ReportError(fC, classId, className,
                  ErrorDefinition::LINT_IMPLEMENT_INTERFACE_CLASS, errors,
                  symbols);
    }
  }
}
