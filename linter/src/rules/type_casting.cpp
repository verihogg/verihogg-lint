#include "rules/type_casting.h"

#include <string_view>
#include <unordered_set>

#include "Surelog/Design/FileContent.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"
#include "Surelog/SourceCompile/VObjectTypes.h"
#include "utils/location_utils.h"
#include "utils/name_utils.h"

using namespace SURELOG;

static std::unordered_set<std::string_view> collectUserDefinedTypes(
    const FileContent* fC, NodeId root) {
  std::unordered_set<std::string_view> userTypes;

  for (NodeId declNode :
       fC->sl_collect_all(root, VObjectType::paType_declaration)) {
    for (NodeId child :
         fC->sl_collect_all(declNode, VObjectType::slStringConst, false)) {
      std::string_view typeName = fC->SymName(child);
      if (!typeName.empty()) userTypes.insert(typeName);
    }
  }

  return userTypes;
}

void checkTypeCasting(const FileContent* fC, ErrorContainer* errors,
                      SymbolTable* symbols) {
  if (!fC || !errors || !symbols) return;

  NodeId root = fC->getRootNode();
  if (!root) return;

  auto userTypes = collectUserDefinedTypes(fC, root);
  if (userTypes.empty()) return;

  for (NodeId funcCallNode :
       fC->sl_collect_all(root, VObjectType::paComplex_func_call)) {
    std::string_view typeName = extractName(fC, funcCallNode);
    if (typeName.empty()) continue;

    if (userTypes.contains(typeName)) {
      reportError(fC, fC->Child(funcCallNode), typeName,
                  ErrorDefinition::LINT_TYPE_CASTING, errors, symbols);
    }
  }
}