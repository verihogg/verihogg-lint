#include "rules/hierarchical_interface_identifier.h"

#include <Surelog/Design/Design.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <string>
#include <string_view>
#include <vector>

#include "utils/location_utils.h"

namespace SL = SURELOG;

static auto JoinNames(const SL::FileContent* fileContent,
                      const std::vector<SL::NodeId>& parts) -> std::string {
  if (parts.empty()) {
    return "<unknown>";
  }
  std::string res;
  bool first = true;
  for (SL::NodeId part : parts) {
    if (!first) {
      res += '.';
    }
    res += std::string(fileContent->SymName(part));
    first = false;
  }
  return res;
}

void CheckHierarchicalInterfaceIdentifier(const SL::FileContent* fileContent,
                                          SL::ErrorContainer* errors,
                                          SL::SymbolTable* symbols) {
  if (fileContent == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }
  SL::NodeId root = fileContent->getRootNode();
  if (root == SL::InvalidNodeId) {
    return;
  }

  for (SL::NodeId iid : fileContent->sl_collect_all(
           root, SL::VObjectType::paInterface_identifier)) {
    auto parts =
        fileContent->sl_collect_all(iid, SL::VObjectType::slStringConst);
    if (parts.size() <= 1) {
      continue;
    }

    ReportError(fileContent, iid, JoinNames(fileContent, parts),
                SL::ErrorDefinition::LINT_HIERARCHICAL_INTERFACE_IDENTIFIER,
                errors, symbols);
  }
}