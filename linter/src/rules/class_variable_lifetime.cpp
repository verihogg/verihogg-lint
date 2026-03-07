#include "rules/class_variable_lifetime.h"

#include <string_view>

#include "Surelog/Design/Design.h"
#include "Surelog/Design/FileContent.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"
#include "Surelog/SourceCompile/VObjectTypes.h"
#include "utils/location_utils.h"
#include "utils/name_utils.h"

using namespace SURELOG;

void checkClassVariableLifetime(const FileContent* fC, ErrorContainer* errors,
                                SymbolTable* symbols) {
  if (!fC || !errors || !symbols) return;
  NodeId root = fC->getRootNode();
  if (!root) return;

  for (NodeId classId :
       fC->sl_collect_all(root, VObjectType::paClass_declaration)) {
    for (NodeId propId :
         fC->sl_collect_all(classId, VObjectType::paClass_property)) {
      for (NodeId autoId :
           fC->sl_collect_all(propId, VObjectType::paLifetime_Automatic)) {
        std::string_view varName = extractVariableName(fC, propId);
        reportError(fC, autoId, varName,
                    ErrorDefinition::LINT_CLASS_VARIABLE_LIFETIME, errors,
                    symbols);
      }
    }
  }
}