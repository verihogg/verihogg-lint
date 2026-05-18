#include "rules/implement_class.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/Design.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <cassert>
#include <string>
#include <unordered_set>
#include <vector>

#include "main/lint_rules.h"
#include "utils/ast_utils.h"
#include "utils/design_utils.h"
#include "utils/location_utils.h"

namespace {

auto GetSuperclassStringFromInterfaceClass(
    const SURELOG::FileContent* fileContent, SURELOG::NodeId node)
    -> std::string {
  SURELOG::NodeId classType = node;
  classType = fileContent->sl_get(classType,
                                  SURELOG::VObjectType::paInterface_class_type);
  classType =
      fileContent->sl_get(classType, SURELOG::VObjectType::paPs_identifier);
  if (!classType) {
    return "";
  }
  return GetStringConst(fileContent, classType);
}

}  // namespace

void CheckImplementClass(SURELOG::Design* design,
                         SURELOG::ErrorContainer* errors,
                         SURELOG::SymbolTable* symbols) {
  if (design == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }

  const std::unordered_set<std::string> interfaceClassSet =
      GetInterfaceClassSet(design);
  const std::unordered_set<std::string> classSet = GetClassSet(design);

  DesignUtils::ForEachFileContent(
      design, [&](const SL::FileContent* fileContent) {
        const std::vector<SURELOG::NodeId> kClassDeclarations =
            fileContent->sl_collect_all(
                fileContent->getRootNode(),
                SURELOG::VObjectType::paClass_declaration);

        for (const auto& classId : kClassDeclarations) {
          const SURELOG::NodeId kImplementsId =
              fileContent->sl_get(classId, SURELOG::VObjectType::paIMPLEMENTS);
          if (!kImplementsId) {
            continue;
          }

          const std::string kSuperclassName =
              GetSuperclassStringFromInterfaceClass(fileContent, classId);

          if (kSuperclassName == "") {
            continue;
          }

          if (classSet.contains(kSuperclassName)) {
            const std::string className = GetStringConst(fileContent, classId);
            ReportError(fileContent, classId, className,
                        verihogg_lint::LINT_IMPLEMENT_CLASS, errors, symbols);
          }
        }
      });
}
