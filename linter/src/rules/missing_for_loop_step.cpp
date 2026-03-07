#include "rules/missing_for_loop_step.h"

#include "Surelog/Design/FileContent.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"
#include "Surelog/SourceCompile/VObjectTypes.h"
#include "utils/ast_utils.h"
#include "utils/location_utils.h"
#include "utils/name_utils.h"

using namespace SURELOG;

void checkMissingForLoopStep(const FileContent* fC, ErrorContainer* errors,
                             SymbolTable* symbols) {
  if (!fC || !errors || !symbols) return;

  NodeId root = fC->getRootNode();
  if (!root) return;

  for (NodeId forNode : fC->sl_collect_all(root, VObjectType::paFOR)) {
    if (hasSiblingOfType(fC, forNode, VObjectType::paFor_step)) continue;

    reportError(fC, forNode, findForLoopVariableName(fC, forNode),
                ErrorDefinition::LINT_MISSING_FOR_LOOP_STEP, errors, symbols);
  }
}
