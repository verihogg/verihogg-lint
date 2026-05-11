#include "rules/extend_class.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/Design.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/Design/ModuleInstance.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <map>
#include <string>
#include <vector>

#include "Surelog/Common/FileSystem.h"
#include "main/lint_rules.h"
#include "utils/ast_utils.h"
#include "utils/design_utils.h"
#include "utils/location_utils.h"

void CheckExtendClass(SL::Design* design, SL::ErrorContainer* errors,
                      SL::SymbolTable* symbols) {
  if (design == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }
  std::map<std::string, std::vector<SL::NodeId>> classMap;

  DesignUtils::ForEachFileContent(
      design, [&](const SL::FileContent* fileContent) {
        const std::vector<SL::NodeId> kClassDeclarations =
            fileContent->sl_collect_all(fileContent->getRootNode(),
                                        SL::VObjectType::paClass_declaration);

        for (const auto& kId : kClassDeclarations) {
          const std::string kClassName = GetStringConst(fileContent, kId);
          classMap[kClassName].push_back(kId);
        }
      });

  DesignUtils::ForEachFileContent(
      design, [&](const SL::FileContent* fileContent) {
        const std::vector<SL::NodeId> kClassDeclarations =
            fileContent->sl_collect_all(fileContent->getRootNode(),
                                        SL::VObjectType::paClass_declaration);
        for (const auto& kId : kClassDeclarations) {
          const SL::NodeId kExtendsId =
              fileContent->sl_get(kId, SL::VObjectType::paEXTENDS);
          if (!kExtendsId) {
            continue;
          }
          const std::string kClassName = GetStringConst(fileContent, kId);
          const std::string kSuperclassName =
              GetSuperclassString(fileContent, kId);

          if (IsBuiltinClass(kClassName) || kSuperclassName == "") {
            continue;
          }

          if (classMap.find(kSuperclassName) == classMap.end()) {
            ReportError(fileContent, kId, kSuperclassName,
                        verihogg_lint::LINT_EXTEND_CLASS, errors, symbols);
          }
        }
      });
}
