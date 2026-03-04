#include "rules/missing_for_loop_condition.h"

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

void checkMissingForLoopCondition(const FileContent* fC, ErrorContainer* errors,
                                  SymbolTable* symbols) {
  if (!fC || !errors || !symbols) return;

  NodeId root = fC->getRootNode();
  if (!root) return;

  auto forNodes = fC->sl_collect_all(root, VObjectType::paFOR);

  for (NodeId forNode : forNodes) {
    if (!forNode) continue;

    bool hasCondition = false;
    for (NodeId tmp = fC->Sibling(forNode); tmp; tmp = fC->Sibling(tmp)) {
      if (fC->Type(tmp) == VObjectType::paExpression) {
        hasCondition = true;
        break;
      }
    }

    if (hasCondition) continue;

    std::string varName = findForLoopVariableName(fC, forNode);

    reportError(fC, forNode, varName,
                ErrorDefinition::LINT_MISSING_FOR_LOOP_CONDITION, errors,
                symbols);
  }
}

