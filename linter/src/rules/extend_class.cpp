#include "rules/extend_class.h"

#include <cassert>
#include <iostream>
#include <vector>

#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/Library/Library.h"
#include "Surelog/Testbench/ClassDefinition.h"
#include "utils/ast_utils.h"
#include "utils/location_utils.h"

using namespace SURELOG;

std::string getSuperclassString(const FileContent* fC, NodeId id) {
  assert(fC->Type(id) != VObjectType::paClass_declaration);

  const NodeId classType = fC->sl_get(id, VObjectType::paClass_type);
  if (classType == NodeId(InvalidRawNodeId)) return "";
  return getStringConst(fC, classType);
}

void checkExtendClass(const FileContent* fC, ErrorContainer* errors,
                      SymbolTable* symbols) {
  if (!fC) return;

  const std::vector<NodeId> classDeclarations =
      fC->sl_collect_all(fC->getRootNode(), VObjectType::paClass_declaration);

  for (auto& id : classDeclarations) {
    const std::string className = getStringConst(fC, id);
    const std::string superclass = getSuperclassString(fC, id);

    if (isBuiltinClass(className) || superclass == "") continue;

    const std::string fullName = getFullName(fC, id);
    const ClassDefinition* def = fC->getClassDefinition(fullName);
    if (def == nullptr)
      ReportError(fC, id, superclass, ErrorDefinition::LINT_EXTEND_CLASS,
                  errors, symbols);
  }
}
