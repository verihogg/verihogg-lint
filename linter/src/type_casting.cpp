#include "type_casting.h"

#include <string>
#include <unordered_set>

#include "Surelog/Design/FileContent.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"
#include "Surelog/SourceCompile/VObjectTypes.h"
#include "linter_utils.h"

using namespace SURELOG;

namespace Analyzer {

static std::unordered_set<std::string> collectUserDefinedTypes(
    const FileContent* fC, NodeId root) {
  std::unordered_set<std::string> userTypes;

  auto typeDecls = fC->sl_collect_all(root, VObjectType::paType_declaration);

  for (NodeId declNode : typeDecls) {
    NodeId child = fC->Child(declNode);
    while (child) {
      if (fC->Type(child) == VObjectType::slStringConst) {
        std::string typeName = std::string(fC->SymName(child));
        if (!typeName.empty()) {
          userTypes.insert(typeName);
        }
      }
      child = fC->Sibling(child);
    }
  }

  return userTypes;
}

void checkTypeCasting(const FileContent* fC, ErrorContainer* errors,
                      SymbolTable* symbols) {
  if (!fC || !errors || !symbols) return;

  NodeId root = fC->getRootNode();

  std::unordered_set<std::string> userTypes = collectUserDefinedTypes(fC, root);

  if (userTypes.empty()) return;

  auto funcCallNodes =
      fC->sl_collect_all(root, VObjectType::paComplex_func_call);

  for (NodeId funcCallNode : funcCallNodes) {
    std::string typeName = extractName(fC, funcCallNode);

    if (typeName.empty()) continue;

    if (userTypes.count(typeName)) {
      NodeId typeNameNode = fC->Child(funcCallNode);
      reportError(fC, typeNameNode, typeName,
                  ErrorDefinition::LINT_TYPE_CASTING, errors, symbols);
    }
  }
}

}  // namespace Analyzer