#include "rules/implement_class.h"

#include <cassert>
#include <string>
#include <unordered_set>
#include <vector>

#include "main/lint_rules.h"
#include "utils/ast_utils.h"
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
  if (classType == kZeroId) {
    return "";
  }
  return GetStringConst(fileContent, classType);
}

}  // namespace

void CheckImplementClass(const SURELOG::FileContent* fileContent,
                         SURELOG::ErrorContainer* errors,
                         SURELOG::SymbolTable* symbols) {
  if (fileContent == nullptr) {
    return;
  }

  std::unordered_set<std::string> interfaceClassSet =
      GetInterfaceClassSet(fileContent);
  std::unordered_set<std::string> classSet = GetClassSet(fileContent);

  const std::vector<SURELOG::NodeId> kClassDeclarations =
      fileContent->sl_collect_all(fileContent->getRootNode(),
                                  SURELOG::VObjectType::paClass_declaration);

  for (const auto& classId : kClassDeclarations) {
    const SURELOG::NodeId kImplementsId =
        fileContent->sl_get(classId, SURELOG::VObjectType::paIMPLEMENTS);
    if (kImplementsId == kZeroId) {
      continue;
    }

    const std::string kSuperclassName =
        GetSuperclassStringFromInterfaceClass(fileContent, classId);

    if (kSuperclassName == "") {
      continue;
    }

    if (classSet.contains(kSuperclassName)) {
      std::string className = GetStringConst(fileContent, classId);
      ReportError(fileContent, classId, className,
                  verihogg_lint::LINT_IMPLEMENT_CLASS, errors, symbols);
    }
  }
}
