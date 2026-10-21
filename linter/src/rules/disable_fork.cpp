#include "rules/disable_fork.h"

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

constexpr SL::VObjectType kPaDisableStatement =
    SL::VObjectType::paDisable_statement;

}  // namespace

void CheckDisableFork(const SURELOG::FileContent* fileContent,
                      SURELOG::ErrorContainer* errors,
                      SURELOG::SymbolTable* symbols) {
  if (fileContent == nullptr) {
    return;
  }

  const std::vector<SL::NodeId> kDisables = fileContent->sl_collect_all(
      fileContent->getRootNode(), kPaDisableStatement);

  for (const auto dis : kDisables) {
    if (fileContent->Child(dis) != SL::InvalidNodeId) {
      continue;
    }

    ReportError(fileContent, dis, "fork", verihogg_lint::LINT_DISABLE_FORK,
                errors, symbols);
  }
}
