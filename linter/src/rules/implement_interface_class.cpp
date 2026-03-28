#include "rules/implement_interface_class.h"

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
#include "utils/location_utils.h"

namespace {
auto GetSuperInterfaceString(const SURELOG::FileContent* fileContent,
                             SURELOG::NodeId node) -> std::string {
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

void CheckImplementInterfaceClass(const SURELOG::FileContent* fileContent,
                                  SURELOG::ErrorContainer* errors,
                                  SURELOG::SymbolTable* symbols) {
  if (fileContent == nullptr) {
    return;
  }

  const std::unordered_set<std::string> classSet = GetClassSet(fileContent);
  const std::unordered_set<std::string> interfaceClassSet =
      GetInterfaceClassSet(fileContent);

  const std::vector<SURELOG::NodeId> kClassDeclarations =
      fileContent->sl_collect_all(fileContent->getRootNode(),
                                  SURELOG::VObjectType::paClass_declaration);

  for (const auto& classId : kClassDeclarations) {
    const SURELOG::NodeId kImplementsId =
        fileContent->sl_get(classId, SURELOG::VObjectType::paIMPLEMENTS);
    if (!kImplementsId) {
      continue;
    }

    const std::string kSuperInterfaceName =
        GetSuperInterfaceString(fileContent, classId);
    if (kSuperInterfaceName == "") {
      continue;
    }

    if (!interfaceClassSet.contains(kSuperInterfaceName)) {
      const std::string className = GetStringConst(fileContent, classId);
      ReportError(fileContent, classId, className,
                  verihogg_lint::LINT_IMPLEMENT_INTERFACE_CLASS, errors,
                  symbols);
    }
  }
}
