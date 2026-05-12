#include "rules/illegal_cunit_reference.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <string_view>

#include "main/lint_rules.h"
#include "utils/location_utils.h"

namespace SL = SURELOG;

namespace {
constexpr std::string_view kUnitToken = "$unit";
}

void CheckIllegalCunitReference(const SL::FileContent* fileContent,
                                SL::ErrorContainer* errors,
                                SL::SymbolTable* symbols) {
  if (fileContent == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }
  SL::NodeId const kRoot = fileContent->getRootNode();
  if (!kRoot) {
    return;
  }

  for (SL::NodeId const kNode :
       fileContent->sl_collect_all(kRoot, SL::VObjectType::slStringConst)) {
    if (fileContent->SymName(kNode) != kUnitToken) {
      continue;
    }
    ReportError(fileContent, kNode, kUnitToken,
                verihogg_lint::LINT_ILLEGAL_CUNIT_REFERENCE, errors, symbols);
  }
}