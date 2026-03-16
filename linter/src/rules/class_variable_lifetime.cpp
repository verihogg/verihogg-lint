#include "rules/class_variable_lifetime.h"

#include <Surelog/Design/Design.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <string_view>

#include "utils/location_utils.h"
#include "utils/name_utils.h"

namespace SL = SURELOG;

void CheckClassVariableLifetime(const SL::FileContent* fileContent,
                                SL::ErrorContainer* errors,
                                SL::SymbolTable* symbols) {
  if (fileContent == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }
  SL::NodeId root = fileContent->getRootNode();
  if (!root) {
    return;
  }

  for (SL::NodeId classId : fileContent->sl_collect_all(
           root, SL::VObjectType::paClass_declaration)) {
    for (SL::NodeId propId : fileContent->sl_collect_all(
             classId, SL::VObjectType::paClass_property)) {
      for (SL::NodeId autoId : fileContent->sl_collect_all(
               propId, SL::VObjectType::paLifetime_Automatic)) {
        std::string_view varName = ExtractVariableName(fileContent, propId);
        ReportError(fileContent, autoId, varName,
                    SL::ErrorDefinition::LINT_CLASS_VARIABLE_LIFETIME, errors,
                    symbols);
      }
    }
  }
}