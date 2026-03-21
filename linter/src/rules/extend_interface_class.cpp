#include "rules/extend_interface_class.h"

#include <cassert>
#include <map>
#include <string_view>
#include <vector>

#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "utils/ast_utils.h"
#include "utils/location_utils.h"

using namespace SURELOG;

namespace {
std::vector<std::string> getSuperclassStrings(const FileContent* fC,
                                              NodeId id) {
  assert(fC->Type(id) != VObjectType::paClass_declaration);

  std::vector<std::string> result;
  const std::vector<NodeId> classType =
      fC->sl_collect_all(id, VObjectType::paInterface_class_type);
  for (auto& id : classType) {
    const NodeId ident = fC->sl_get(id, VObjectType::paPs_identifier);
    std::string superName = (ident == zeroId) ? "" : getStringConst(fC, ident);
    result.push_back(superName);
  }

  return result;
}
}  // namespace

void checkExtendInterfaceClass(const FileContent* fC, ErrorContainer* errors,
                               SymbolTable* symbols) {
  if (!fC) return;

  std::cout << fC->printSubTree(fC->getRootNode()) << std::endl;
  const NodeId rootNode = fC->getRootNode();
  const std::vector<NodeId> interfaceClassDeclarations =
      fC->sl_collect_all(rootNode, VObjectType::paInterface_class_declaration);

  std::map<std::string, std::vector<NodeId>> interfaceClassMap;
  for (auto& id : interfaceClassDeclarations) {
    const std::string className = getStringConst(fC, id);
    interfaceClassMap[className].push_back(id);
  }

  for (auto& interfaceId : interfaceClassDeclarations) {
    const std::string className = getStringConst(fC, interfaceId);
    const std::string mainPrefix = getPrefix(fC, interfaceId);
    const std::vector<std::string> superclasses =
        getSuperclassStrings(fC, interfaceId);

    for (auto& superclassName : superclasses) {
      if (isBuiltinClass(className) || superclassName == "") continue;

      std::vector<NodeId> superIdVector = interfaceClassMap[superclassName];
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

      ReportError(fC, interfaceId, className,
                  ErrorDefinition::LINT_EXTEND_INTERFACE_CLASS, errors,
                  symbols);
    }
  }
}
