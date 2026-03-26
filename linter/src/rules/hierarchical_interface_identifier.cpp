#include "rules/hierarchical_interface_identifier.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/Design.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <string>
#include <vector>

#include "main/lint_rules.h"
#include "utils/location_utils.h"

namespace SL = SURELOG;

namespace {
auto JoinNames(const SL::FileContent* fileContent,
               const std::vector<SL::NodeId>& parts) -> std::string {
  if (parts.empty()) {
    return "<unknown>";
  }
  std::string res;
  bool first = true;
  for (SL::NodeId const kPart : parts) {
    if (!first) {
      res += '.';
    }
    res += std::string(fileContent->SymName(kPart));
    first = false;
  }
  return res;
}
}  // namespace

void CheckHierarchicalInterfaceIdentifier(const SL::FileContent* fileContent,
                                          SL::ErrorContainer* errors,
                                          SL::SymbolTable* symbols) {
  if (fileContent == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }
  SL::NodeId const kRoot = fileContent->getRootNode();
  if (kRoot == SL::InvalidNodeId) {
    return;
  }

  for (SL::NodeId const kIid : fileContent->sl_collect_all(
           kRoot, SL::VObjectType::paInterface_identifier)) {
    auto parts =
        fileContent->sl_collect_all(kIid, SL::VObjectType::slStringConst);
    if (parts.size() <= 1) {
      continue;
    }

    ReportError(fileContent, kIid, JoinNames(fileContent, parts),
                verihogg_lint::LINT_HIERARCHICAL_INTERFACE_IDENTIFIER, errors,
                symbols);
  }
}