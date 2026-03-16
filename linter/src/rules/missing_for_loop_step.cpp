#include "rules/missing_for_loop_step.h"

#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include "utils/ast_utils.h"
#include "utils/location_utils.h"
#include "utils/name_utils.h"

namespace SL = SURELOG;

void CheckMissingForLoopStep(const SL::FileContent* fileContent,
                             SL::ErrorContainer* errors,
                             SL::SymbolTable* symbols) {
  if (fileContent == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }

  SL::NodeId root = fileContent->getRootNode();
  if (!root) {
    return;
  }

  for (SL::NodeId forNode :
       fileContent->sl_collect_all(root, SL::VObjectType::paFOR)) {
    if (HasSiblingOfType(fileContent, forNode, SL::VObjectType::paFor_step)) {
      continue;
    }

    ReportError(
        fileContent, forNode, FindForLoopVariableName(fileContent, forNode),
        SL::ErrorDefinition::LINT_MISSING_FOR_LOOP_STEP, errors, symbols);
  }
}
