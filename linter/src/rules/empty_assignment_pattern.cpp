#include "rules/empty_assignment_pattern.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/Design.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/ErrorReporting/ErrorDefinition.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <string_view>

#include "utils/location_utils.h"
#include "utils/name_utils.h"

namespace SL = SURELOG;

void CheckEmptyAssignmentPattern(const SL::FileContent* fileContent,
                                 SL::ErrorContainer* errors,
                                 SL::SymbolTable* symbols) {
  if (fileContent == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }
  SL::NodeId const root = fileContent->getRootNode();
  if (!root) {
    return;
  }

  for (SL::NodeId const pat : fileContent->sl_collect_all(
           root, SL::VObjectType::paAssignment_pattern)) {
    if (!pat) {
      continue;
    }

    if (fileContent->Child(pat)) {
      continue;
    }

    std::string_view const varName = FindDirectRhsLhsName(fileContent, pat);

    ReportError(fileContent, pat, varName,
                SL::ErrorDefinition::LINT_EMPTY_ASSIGNMENT_PATTERN, errors,
                symbols);
  }
}
