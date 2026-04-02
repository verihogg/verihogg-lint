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

  for (SL::NodeId const kSeqExprId :
       fileContent->sl_collect_all(kRoot, SL::VObjectType::paSequence_expr)) {
    const bool hasGoto =
        !fileContent
             ->sl_collect_all(kSeqExprId, SL::VObjectType::paGoto_repetition)
             .empty();

    const bool hasNonConsecutive =
        !fileContent
             ->sl_collect_all(kSeqExprId,
                              SL::VObjectType::paNon_consecutive_repetition)
             .empty();

    if (hasGoto || hasNonConsecutive) {
      std::string_view name = "<unknown>";

      SL::NodeId parent = kSeqExprId;
      while (parent) {
        if (fileContent->Type(parent) ==
            SL::VObjectType::paProperty_declaration) {
          name = ExtractName(fileContent, parent);
          break;
        }
        parent = fileContent->Parent(parent);
      }

      ReportError(fileContent, kSeqExprId, name,
                  verihogg_lint::LINT_REPETITION_IN_SEQUENCE, errors, symbols);
    }
  }
}
