#include "rules/empty_assignment_pattern.h"

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

void CheckEmptyAssignmentPattern(const SL::FileContent* fileContent,
                                 SL::ErrorContainer* errors,
                                 SL::SymbolTable* symbols) {
  if (fileContent == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }
  SL::NodeId const kRoot = fileContent->getRootNode();
  if (!kRoot) {
    return;
  }

  for (SL::NodeId const kPat : fileContent->sl_collect_all(
           kRoot, SL::VObjectType::paAssignment_pattern)) {
    if (!kPat) {
      continue;
    }

    if (fileContent->Child(kPat)) {
      continue;
    }

    std::string_view const kVarName = FindDirectRhsLhsName(fileContent, kPat);

    ReportError(fileContent, kPat, kVarName,
                verihogg_lint::LINT_EMPTY_ASSIGNMENT_PATTERN, errors, symbols);
  }
}
