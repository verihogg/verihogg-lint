#include "rules/duplicate_constructor.h"

#include <cassert>
#include <map>
#include <vector>

#include "main/lint_rules.h"
#include "utils/ast_utils.h"
#include "utils/location_utils.h"

void CheckDuplicateConstructor(const SURELOG::FileContent* fileContent,
                               SURELOG::ErrorContainer* errors,
                               SURELOG::SymbolTable* symbols) {
  if (fileContent == nullptr) {
    return;
  }

  const std::vector<SURELOG::NodeId> kClassDeclarations =
      fileContent->sl_collect_all(fileContent->getRootNode(),
                                  VObjectType::paClass_declaration);

  for (const auto& classId : kClassDeclarations) {
    const std::string kClassName = GetStringConst(fileContent, classId);
    std::map<std::vector<VObjectType>, bool> argTree;
    std::vector<SURELOG::NodeId> methods =
        fileContent->sl_get_all(classId, VObjectType::paClass_item);

    for (auto& methodId : methods) {
      methodId = fileContent->sl_get(methodId, VObjectType::paClass_method);
      methodId = fileContent->sl_get(
          methodId, VObjectType::paClass_constructor_declaration);
      if (methodId == kZeroId) {
        continue;
      }

      std::vector<VObjectType> types;
      std::vector<SURELOG::NodeId> arguments;

      methodId = fileContent->sl_get(methodId, VObjectType::paTf_port_list);
      std::vector<SURELOG::NodeId> items =
          fileContent->sl_get_all(methodId, VObjectType::paTf_port_item);
      for (auto& item : items) {
        item = fileContent->sl_get(item, VObjectType::paData_type_or_implicit);
        item = fileContent->sl_get(item, VObjectType::paData_type);
        item = fileContent->Child(item);
        if (item == kZeroId) {
          continue;
        }

        const VObjectType kType = fileContent->Type(item);
        types.push_back(kType);
      }

      if (argTree.contains(types)) {
        ReportError(fileContent, methodId, kClassName,
                    verihogg_lint::LINT_DUPLICATE_CONSTRUCTOR, errors, symbols);
      }
      argTree[types] = true;
    }
  }
}
