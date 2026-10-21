#include "rules/named_fork.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <vector>

#include "main/lint_rules.h"
#include "utils/location_utils.h"

namespace {

namespace SL = SURELOG;

bool IsForkLabel(SL::VObjectType t) {
  return t == SL::VObjectType::slStringConst;
}

bool HasForkLabel(const SL::FileContent* fC, SL::NodeId parBlock) {
  const SL::NodeId first = fC->Child(parBlock);
  return first != SL::InvalidNodeId && IsForkLabel(fC->Type(first));
}

}  // namespace

void CheckNamedFork(const SURELOG::FileContent* fileContent,
                    SURELOG::ErrorContainer* errors,
                    SURELOG::SymbolTable* symbols) {
  if (fileContent == nullptr) {
    return;
  }

  const std::vector<SL::NodeId> kParBlocks = fileContent->sl_collect_all(
      fileContent->getRootNode(), SL::VObjectType::paPar_block);

  for (const auto parBlock : kParBlocks) {
    if (!HasForkLabel(fileContent, parBlock)) {
      ReportError(fileContent, parBlock, "fork", verihogg_lint::LINT_NAMED_FORK,
                  errors, symbols);
    }
  }
}
