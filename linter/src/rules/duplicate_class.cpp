#include "rules/duplicate_class.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/Design.h>
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

void CheckDuplicateClass(const SURELOG::FileContent* fileContent,
                         SURELOG::ErrorContainer* errors,
                         SURELOG::SymbolTable* symbols) {
  if (fileContent == nullptr) {
    return;
  }

  const std::vector<SURELOG::NodeId> kClassDeclarations =
      fileContent->sl_collect_all(fileContent->getRootNode(),
                                  SURELOG::VObjectType::paClass_declaration);

  std::unordered_set<std::string> seenSet;
  std::unordered_set<std::string> duplicateSet;

  for (const auto& classId : kClassDeclarations) {
    const std::string kFullName = GetFullName(fileContent, classId);
    const bool kIsSeen = seenSet.contains(kFullName);
    const bool kIsDuplicate = duplicateSet.contains(kFullName);

    if (kIsDuplicate) {
      duplicateSet.erase(kFullName);
    } else if (kIsSeen) {
      duplicateSet.insert(kFullName);
      ReportError(fileContent, classId, kFullName,
                  verihogg_lint::LINT_DUPLICATE_CLASS, errors, symbols);
    } else {
      seenSet.insert(kFullName);
    }
  }
}
