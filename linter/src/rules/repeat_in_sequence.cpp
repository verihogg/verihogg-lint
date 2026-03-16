#include "rules/repeat_in_sequence.h"

#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <string_view>

#include "utils/location_utils.h"
#include "utils/name_utils.h"

namespace SL = SURELOG;

void CheckRepetitionInSequence(const SL::FileContent* fileContent,
                               SL::ErrorContainer* errors,
                               SL::SymbolTable* symbols) {
  if (fileContent == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }

  SL::NodeId root = fileContent->getRootNode();
  if (!root) {
    return;
  }

  for (SL::NodeId seqDeclId : fileContent->sl_collect_all(
           root, SL::VObjectType::paSequence_declaration)) {
    std::string_view seqName = ExtractName(fileContent, seqDeclId);

    for (SL::NodeId seqExprId : fileContent->sl_collect_all(
             seqDeclId, SL::VObjectType::paSequence_expr)) {
      if (fileContent
              ->sl_collect_all(seqExprId, SL::VObjectType::paGoto_repetition)
              .empty()) {
        continue;
      }
      if (fileContent
              ->sl_collect_all(seqExprId,
                               SL::VObjectType::paNon_consecutive_repetition)
              .empty()) {
        continue;
      }

      ReportError(fileContent, seqExprId, seqName,
                  SL::ErrorDefinition::LINT_REPETITION_IN_SEQUENCE, errors,
                  symbols);
    }
  }
}
