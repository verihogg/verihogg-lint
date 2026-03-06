#include "rules/empty_assignment_pattern.h"

#include <cstdint>
#include <string>

#include "Surelog/Design/Design.h"
#include "Surelog/Design/FileContent.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"
#include "Surelog/SourceCompile/VObjectTypes.h"
#include "utils/location_utils.h"
#include "utils/name_utils.h"

using namespace SURELOG;

void checkEmptyAssignmentPattern(const FileContent* fC, ErrorContainer* errors,
                                 SymbolTable* symbols) {
  if (!fC || !errors || !symbols) return;
  NodeId root = fC->getRootNode();
  if (!root) return;

  for (NodeId pat :
       fC->sl_collect_all(root, VObjectType::paAssignment_pattern)) {
    if (!pat) continue;

    if (fC->Child(pat)) continue;

    std::string varName = findDirectRhsLhsName(fC, pat);

    reportError(fC, pat, varName,
                ErrorDefinition::LINT_EMPTY_ASSIGNMENT_PATTERN, errors,
                symbols);
  }
}
