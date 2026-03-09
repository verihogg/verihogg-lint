#include "rules/hierarchical_interface_identifier.h"

#include <string>
#include <string_view>
#include <vector>

#include "Surelog/Design/Design.h"
#include "Surelog/Design/FileContent.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"
#include "Surelog/SourceCompile/VObjectTypes.h"
#include "utils/location_utils.h"

using namespace SURELOG;

static std::string JoinNames(const FileContent* fC,
                             const std::vector<NodeId>& parts) {
  if (parts.empty()) return "<unknown>";
  std::string res;
  bool first = true;
  for (NodeId part : parts) {
    if (!first) res += '.';
    res += std::string(fC->SymName(part));
    first = false;
  }
  return res;
}

void CheckHierarchicalInterfaceIdentifier(const FileContent* fC,
                                          ErrorContainer* errors,
                                          SymbolTable* symbols) {
  if (!fC || !errors || !symbols) return;
  NodeId root = fC->getRootNode();
  if (!root) return;

  for (NodeId iid :
       fC->sl_collect_all(root, VObjectType::paInterface_identifier)) {
    auto parts = fC->sl_collect_all(iid, VObjectType::slStringConst);
    if (parts.size() <= 1) continue;

    ReportError(fC, iid, JoinNames(fC, parts),
                ErrorDefinition::LINT_HIERARCHICAL_INTERFACE_IDENTIFIER, errors,
                symbols);
  }
}