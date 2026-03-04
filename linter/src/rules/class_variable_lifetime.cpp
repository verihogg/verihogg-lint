#include "rules/class_variable_lifetime.h"

#include <cstdint>
#include <string>

#include "Surelog/Design/Design.h"
#include "Surelog/Design/FileContent.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"
#include "Surelog/SourceCompile/VObjectTypes.h"
#include "utils/name_utils.h"
#include "utils/location_utils.h"

using namespace SURELOG;

std::string findVariableName(const FileContent* fC, NodeId propId) {
  return extractVariableName(fC, propId);
}

void checkClassVariableLifetime(const FileContent* fC, ErrorContainer* errors,
                                SymbolTable* symbols) {
  NodeId root = fC->getRootNode();

  auto classNodes = fC->sl_collect_all(root, VObjectType::paClass_declaration);

  for (NodeId classId : classNodes) {
    auto classProps =
        fC->sl_collect_all(classId, VObjectType::paClass_property);

    for (NodeId propId : classProps) {
      auto autoNodes =
          fC->sl_collect_all(propId, VObjectType::paLifetime_Automatic);

      for (NodeId autoId : autoNodes) {
        std::string varName = findVariableName(fC, propId);
        reportError(fC, autoId, varName,
                    ErrorDefinition::LINT_CLASS_VARIABLE_LIFETIME, errors,
                    symbols);
      }
    }
  }
}