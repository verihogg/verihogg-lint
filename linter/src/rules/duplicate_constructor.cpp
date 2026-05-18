#include "rules/duplicate_constructor.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/Design.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <cassert>
#include <map>
#include <string>
#include <vector>

#include "main/lint_rules.h"
#include "utils/ast_utils.h"
#include "utils/design_utils.h"
#include "utils/location_utils.h"

void CheckDuplicateConstructor(SURELOG::Design* design,
                               SURELOG::ErrorContainer* errors,
                               SURELOG::SymbolTable* symbols) {
  if (design == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }

  DesignUtils::ForEachFileContent(
      design, [&](const SL::FileContent* fileContent) {
        const std::vector<SURELOG::NodeId> kClassDeclarations =
            fileContent->sl_collect_all(
                fileContent->getRootNode(),
                SURELOG::VObjectType::paClass_declaration);

        for (const auto& classId : kClassDeclarations) {
          const std::string kClassName = GetStringConst(fileContent, classId);
          std::map<std::vector<SURELOG::VObjectType>, bool> argTree;
          std::vector<SURELOG::NodeId> methods = fileContent->sl_get_all(
              classId, SURELOG::VObjectType::paClass_item);

          for (auto& methodId : methods) {
            methodId = fileContent->sl_get(
                methodId, SURELOG::VObjectType::paClass_method);
            methodId = fileContent->sl_get(
                methodId,
                SURELOG::VObjectType::paClass_constructor_declaration);
            if (!methodId) {
              continue;
            }

            std::vector<SURELOG::VObjectType> types;
            const std::vector<SURELOG::NodeId> arguments;

            methodId = fileContent->sl_get(
                methodId, SURELOG::VObjectType::paTf_port_list);
            std::vector<SURELOG::NodeId> items = fileContent->sl_get_all(
                methodId, SURELOG::VObjectType::paTf_port_item);
            for (auto& item : items) {
              item = fileContent->sl_get(
                  item, SURELOG::VObjectType::paData_type_or_implicit);
              item =
                  fileContent->sl_get(item, SURELOG::VObjectType::paData_type);
              item = fileContent->Child(item);
              if (!item) {
                continue;
              }

              const SURELOG::VObjectType kType = fileContent->Type(item);
              types.push_back(kType);
            }

            if (argTree.contains(types)) {
              ReportError(fileContent, methodId, kClassName,
                          verihogg_lint::LINT_DUPLICATE_CONSTRUCTOR, errors,
                          symbols);
            }
            argTree[types] = true;
          }
        }
      });
}
