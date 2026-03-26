#include "rules/type_casting.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <string_view>
#include <unordered_set>

#include "main/lint_rules.h"
#include "utils/location_utils.h"
#include "utils/name_utils.h"

namespace SL = SURELOG;

namespace {
auto CollectUserDefinedTypes(const SL::FileContent* fileContent,
                             SL::NodeId root)
    -> std::unordered_set<std::string_view> {
  std::unordered_set<std::string_view> userTypes;

  for (SL::NodeId const kDeclNode :
       fileContent->sl_collect_all(root, SL::VObjectType::paType_declaration)) {
    for (SL::NodeId const kChild : fileContent->sl_collect_all(
             kDeclNode, SL::VObjectType::slStringConst, false)) {
      std::string_view const kTypeName = fileContent->SymName(kChild);
      if (!kTypeName.empty()) {
        userTypes.insert(kTypeName);
      }
    }
  }

  return userTypes;
}
}  // namespace

void CheckTypeCasting(const SL::FileContent* fileContent,
                      SL::ErrorContainer* errors, SL::SymbolTable* symbols) {
  if (fileContent == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }

  SL::NodeId const kRoot = fileContent->getRootNode();
  if (!kRoot) {
    return;
  }

  auto userTypes = CollectUserDefinedTypes(fileContent, kRoot);
  if (userTypes.empty()) {
    return;
  }

  for (SL::NodeId const kFuncCallNode : fileContent->sl_collect_all(
           kRoot, SL::VObjectType::paComplex_func_call)) {
    std::string_view const kTypeName = ExtractName(fileContent, kFuncCallNode);
    if (kTypeName.empty()) {
      continue;
    }

    if (userTypes.contains(kTypeName)) {
      ReportError(fileContent, fileContent->Child(kFuncCallNode), kTypeName,
                  verihogg_lint::LINT_TYPE_CASTING, errors, symbols);
    }
  }
}