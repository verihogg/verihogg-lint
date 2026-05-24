#include "rules/duplicate_checker.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <string>
#include <unordered_set>
#include <vector>

#include "main/lint_rules.h"
#include "utils/ast_utils.h"
#include "utils/location_utils.h"

void CheckDuplicateChecker(const SURELOG::FileContent* fileContent,
                           SURELOG::ErrorContainer* errors,
                           SURELOG::SymbolTable* symbols) {
  if (fileContent == nullptr) {
    return;
  }

  const std::vector<SURELOG::NodeId> checkerDecls = fileContent->sl_collect_all(
      fileContent->getRootNode(), SURELOG::VObjectType::paChecker_declaration);

  std::unordered_set<std::string> seenSet;
  std::unordered_set<std::string> duplicateSet;

  for (const auto& checkerId : checkerDecls) {
    const std::string fullName = GetFullName(fileContent, checkerId);
    const bool isSeen = seenSet.contains(fullName);
    const bool isDuplicate = duplicateSet.contains(fullName);

    if (isDuplicate) {
      duplicateSet.erase(fullName);
    } else if (isSeen) {
      duplicateSet.insert(fullName);
      ReportError(fileContent, checkerId, fullName,
                  verihogg_lint::LINT_DUPLICATE_CHECKER, errors, symbols);
    } else {
      seenSet.insert(fullName);
    }
  }
}
