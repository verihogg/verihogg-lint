#include "rules/class_variable_lifetime.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/Design.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <string_view>

#include "main/lint_rules.h"
#include "utils/location_utils.h"
#include "utils/name_utils.h"

namespace SL = SURELOG;

void CheckClassVariableLifetime(const SL::FileContent* fileContent,
                                SL::ErrorContainer* errors,
                                SL::SymbolTable* symbols) {
  if (fileContent == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }
  SL::NodeId const kRoot = fileContent->getRootNode();
  if (!kRoot) {
    return;
  }

  for (SL::NodeId const kClassId : fileContent->sl_collect_all(
           kRoot, SL::VObjectType::paClass_declaration)) {
    for (SL::NodeId const kPropId : fileContent->sl_collect_all(
             kClassId, SL::VObjectType::paClass_property)) {
      for (SL::NodeId const kAutoId : fileContent->sl_collect_all(
               kPropId, SL::VObjectType::paLifetime_Automatic)) {
        std::string_view const kVarName =
            ExtractVariableName(fileContent, kPropId);
        ReportError(fileContent, kAutoId, kVarName,
                    verihogg_lint::LINT_CLASS_VARIABLE_LIFETIME, errors,
                    symbols);
      }
    }
  }
}