#include "rules/repeat_in_sequence.h"

#include <cstdint>
#include <string>

#include "Surelog/Design/Design.h"
#include "Surelog/Design/FileContent.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"
#include "Surelog/SourceCompile/VObjectTypes.h"
#include "utils/name_utils.h"
#include "utils/location_utils.h"

using namespace SURELOG;

void checkRepetitionInSequence(const FileContent* fC, ErrorContainer* errors,
                               SymbolTable* symbols) {
  if (!fC) return;

  NodeId root = fC->getRootNode();
  if (!root) return;

  auto seqDecls = fC->sl_collect_all(root, VObjectType::paSequence_declaration);

  for (NodeId seqDeclId : seqDecls) {
    std::string seqName = extractName(fC, seqDeclId);

    auto seqExprs = fC->sl_collect_all(seqDeclId, VObjectType::paSequence_expr);

    for (NodeId seqExprId : seqExprs) {
      auto gotoNodes =
          fC->sl_collect_all(seqExprId, VObjectType::paGoto_repetition);
      auto nonConsecNodes = fC->sl_collect_all(
          seqExprId, VObjectType::paNon_consecutive_repetition);

      bool hasGoto = !gotoNodes.empty();
      bool hasNonConsec = !nonConsecNodes.empty();

      if (hasGoto && hasNonConsec) {
        reportError(fC, seqExprId, seqName,
                    ErrorDefinition::LINT_REPETITION_IN_SEQUENCE, errors,
                    symbols);
      }
    }
  }
}

