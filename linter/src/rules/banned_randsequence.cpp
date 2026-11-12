#include "rules/banned_randsequence.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <string>

#include "main/lint_rules.h"
#include "utils/ast_utils.h"
#include "utils/location_utils.h"

namespace SL = SURELOG;

void CheckBannedRandsequence(const SL::FileContent* fileContent,
                             SL::ErrorContainer* errors,
                             SL::SymbolTable* symbols) {
  if (fileContent == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }

  SL::NodeId const kRoot = fileContent->getRootNode();
  if (!kRoot) {
    return;
  }

  for (SL::NodeId const kRandseqId : fileContent->sl_collect_all(
           kRoot, SL::VObjectType::paRandsequence_statement)) {
    std::string const kProductionName = GetStringConst(fileContent, kRandseqId);
    ReportError(fileContent, kRandseqId, kProductionName,
                verihogg_lint::LINT_BANNED_RANDSEQUENCE, errors, symbols);
  }
}
