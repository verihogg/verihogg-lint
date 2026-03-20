#include "rules/extend_class.h"

#include <cassert>
#include <map>
#include <string_view>
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

  std::map<std::string, std::vector<NodeId>> classMap;
  for (auto& id : classDeclarations) {
    const std::string className = getStringConst(fC, id);
    classMap[className].push_back(id);
  }

  for (auto& id : classDeclarations) {
    const std::string className = getStringConst(fC, id);
    const std::string mainPrefix = getPrefix(fC, id);
    const std::string superclassName = getSuperclassString(fC, id);

    if (isBuiltinClass(className) || superclassName == "") continue;

    std::vector<NodeId> superIdVector = classMap[superclassName];
    bool found = false;
    for (auto& superId : superIdVector) {
      const std::string superPrefix = getPrefix(fC, superId);

      const size_t superSize = superPrefix.size();
      const size_t mainSize = superPrefix.size();
      if (superSize <= mainSize &&
          std::string_view(mainPrefix).substr(0, superSize) == superPrefix) {
        found = true;
        break;
      }
    }

    if (found) continue;

    ReportError(fC, id, superclassName, ErrorDefinition::LINT_EXTEND_CLASS,
                errors, symbols);
  }
}
