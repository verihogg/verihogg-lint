#include "rules/repeat_in_sequence.h"

#include <string_view>

#include "Surelog/Design/FileContent.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"
#include "Surelog/SourceCompile/VObjectTypes.h"
#include "utils/location_utils.h"
#include "utils/name_utils.h"

using namespace SURELOG;

void checkRepetitionInSequence(const FileContent* fC, ErrorContainer* errors,
                               SymbolTable* symbols) {
  if (!fC || !errors || !symbols) return;

  NodeId root = fC->getRootNode();
  if (!root) return;

  for (NodeId seqDeclId :
       fC->sl_collect_all(root, VObjectType::paSequence_declaration)) {
    std::string_view seqName = extractName(fC, seqDeclId);

    for (NodeId seqExprId :
         fC->sl_collect_all(seqDeclId, VObjectType::paSequence_expr)) {
      if (fC->sl_collect_all(seqExprId, VObjectType::paGoto_repetition).empty())
        continue;
      if (fC->sl_collect_all(seqExprId,
                             VObjectType::paNon_consecutive_repetition)
              .empty())
        continue;

      reportError(fC, seqExprId, seqName,
                  ErrorDefinition::LINT_REPETITION_IN_SEQUENCE, errors,
                  symbols);
    }
  }
}
