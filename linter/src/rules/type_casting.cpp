#include "rules/type_casting.h"

#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <string_view>
#include <unordered_set>

#include "utils/location_utils.h"
#include "utils/name_utils.h"

namespace SL = SURELOG;

static auto CollectUserDefinedTypes(const SL::FileContent* fileContent,
                                    SL::NodeId root)
    -> std::unordered_set<std::string_view> {
  std::unordered_set<std::string_view> userTypes;

  for (SL::NodeId declNode :
       fileContent->sl_collect_all(root, SL::VObjectType::paType_declaration)) {
    for (SL::NodeId child : fileContent->sl_collect_all(
             declNode, SL::VObjectType::slStringConst, false)) {
      std::string_view typeName = fileContent->SymName(child);
      if (!typeName.empty()) {
        userTypes.insert(typeName);
      }
    }
  }

  return userTypes;
}

void CheckTypeCasting(const SL::FileContent* fileContent,
                      SL::ErrorContainer* errors, SL::SymbolTable* symbols) {
  if (fileContent == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }

  SL::NodeId root = fileContent->getRootNode();
  if (!root) {
    return;
  }

  auto userTypes = CollectUserDefinedTypes(fileContent, root);
  if (userTypes.empty()) {
    return;
  }

  for (SL::NodeId funcCallNode : fileContent->sl_collect_all(
           root, SL::VObjectType::paComplex_func_call)) {
    std::string_view typeName = ExtractName(fileContent, funcCallNode);
    if (typeName.empty()) {
      continue;
    }

    if (userTypes.contains(typeName)) {
      ReportError(fileContent, fileContent->Child(funcCallNode), typeName,
                  SL::ErrorDefinition::LINT_TYPE_CASTING, errors, symbols);
    }
  }
}