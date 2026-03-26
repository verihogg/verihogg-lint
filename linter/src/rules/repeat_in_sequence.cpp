#include "rules/repeat_in_sequence.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <string_view>

#include "main/lint_rules.h"
#include "utils/location_utils.h"
#include "utils/name_utils.h"

namespace SL = SURELOG;

void CheckRepetitionInSequence(const SL::FileContent* fileContent,
                               SL::ErrorContainer* errors,
                               SL::SymbolTable* symbols) {
  if (fileContent == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }

  SL::NodeId const kRoot = fileContent->getRootNode();
  if (!kRoot) {
    return;
  }

  for (SL::NodeId const kSeqDeclId : fileContent->sl_collect_all(
           kRoot, SL::VObjectType::paSequence_declaration)) {
    std::string_view const kSeqName = ExtractName(fileContent, kSeqDeclId);

    for (SL::NodeId const kSeqExprId : fileContent->sl_collect_all(
             kSeqDeclId, SL::VObjectType::paSequence_expr)) {
      if (fileContent
              ->sl_collect_all(kSeqExprId, SL::VObjectType::paGoto_repetition)
              .empty()) {
        continue;
      }
      if (fileContent
              ->sl_collect_all(kSeqExprId,
                               SL::VObjectType::paNon_consecutive_repetition)
              .empty()) {
        continue;
      }

      ReportError(fileContent, kSeqExprId, kSeqName,
                  verihogg_lint::LINT_REPETITION_IN_SEQUENCE, errors, symbols);
    }
  }
}
