#include "rules/missing_for_loop_step.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include "main/lint_rules.h"
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

  SL::NodeId const kRoot = fileContent->getRootNode();
  if (!kRoot) {
    return;
  }

  for (SL::NodeId const kForNode :
       fileContent->sl_collect_all(kRoot, SL::VObjectType::paFOR)) {
    if (HasSiblingOfType(fileContent, kForNode, SL::VObjectType::paFor_step)) {
      continue;
    }

    ReportError(fileContent, kForNode,
                FindForLoopVariableName(fileContent, kForNode),
                verihogg_lint::LINT_MISSING_FOR_LOOP_STEP, errors, symbols);
  }
}
