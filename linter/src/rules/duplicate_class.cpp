#include "rules/duplicate_class.h"

#include <unordered_set>

#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "utils/ast_utils.h"
#include "utils/location_utils.h"

using namespace SURELOG;

void checkDuplicateClass(const FileContent* fC, ErrorContainer* errors,
                         SymbolTable* symbols) {
  if (!fC) return;

  const std::vector<NodeId> classDeclarations =
      fC->sl_collect_all(fC->getRootNode(), VObjectType::paClass_declaration);

  std::unordered_set<std::string> seenSet;
  std::unordered_set<std::string> duplicateSet;

  for (auto& classId : classDeclarations) {
    const std::string fullName = getFullName(fC, classId);
    const bool isSeen = seenSet.count(fullName) > 0;
    const bool isDuplicate = duplicateSet.count(fullName) > 0;

    if (isDuplicate) {
      duplicateSet.erase(fullName);
    } else if (isSeen) {
      duplicateSet.insert(fullName);
      ReportError(fC, classId, fullName, ErrorDefinition::LINT_DUPLICATE_CLASS,
                  errors, symbols);
    } else {
      seenSet.insert(fullName);
    }
  }
}
