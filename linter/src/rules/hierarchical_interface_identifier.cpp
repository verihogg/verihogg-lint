#include "rules/hierarchical_interface_identifier.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/Design.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <queue>
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

auto IsHierarchicalExpression(const SL::FileContent* fileContent,
                              SL::NodeId expr) -> bool {
  auto ids = fileContent->sl_collect_all(expr, SL::VObjectType::slStringConst);
  if (ids.size() <= 1) {
    return false;
  }

  SL::NodeId const lastId = ids.back();
  auto lastType = fileContent->Type(lastId);

  if (lastType == SL::VObjectType::slStringConst) {
    auto parent = fileContent->Child(lastId);
    if (parent != SL::InvalidNodeId) {
      return true;
    }
    return false;
  }

  return true;
}

void CheckExpressionsIteratively(const SL::FileContent* fileContent,
                                 SL::NodeId root, SL::ErrorContainer* errors,
                                 SL::SymbolTable* symbols) {
  std::queue<SL::NodeId> queue;
  queue.push(root);

  while (!queue.empty()) {
    SL::NodeId const node = queue.front();
    queue.pop();

    if (IsHierarchicalExpression(fileContent, node)) {
      auto parts =
          fileContent->sl_collect_all(node, SL::VObjectType::slStringConst);
      ReportError(fileContent, node, JoinNames(fileContent, parts),
                  verihogg_lint::LINT_HIERARCHICAL_INTERFACE_IDENTIFIER, errors,
                  symbols);
    }

    auto children =
        fileContent->sl_collect_all(node, SL::VObjectType::paExpression);
    for (SL::NodeId const child : children) {
      queue.push(child);
    }
  }
}

void CheckNamedPortConnectionExpressions(const SL::FileContent* fileContent,
                                         SL::NodeId npc,
                                         SL::ErrorContainer* errors,
                                         SL::SymbolTable* symbols) {
  using VT = SL::VObjectType;
  auto exprs = fileContent->sl_collect_all(npc, VT::paExpression);
  for (SL::NodeId const expr : exprs) {
    auto ids = fileContent->sl_collect_all(expr, VT::slStringConst);
    if (ids.size() > 1) {
      ReportError(fileContent, expr, JoinNames(fileContent, ids),
                  verihogg_lint::LINT_HIERARCHICAL_INTERFACE_IDENTIFIER, errors,
                  symbols);
    }
  }
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
    if (parts.size() > 1) {
      ReportError(fileContent, kIid, JoinNames(fileContent, parts),
                  verihogg_lint::LINT_HIERARCHICAL_INTERFACE_IDENTIFIER, errors,
                  symbols);
    }
  }

  for (SL::NodeId const kNpc : fileContent->sl_collect_all(
           kRoot, SL::VObjectType::paNamed_port_connection)) {
    CheckNamedPortConnectionExpressions(fileContent, kNpc, errors, symbols);
  }

  std::vector<SL::NodeId> proceduralBlocks =
      fileContent->sl_collect_all(kRoot, SL::VObjectType::paInitial_construct);
  auto alwaysBlocks =
      fileContent->sl_collect_all(kRoot, SL::VObjectType::paAlways_construct);
  proceduralBlocks.insert(proceduralBlocks.end(), alwaysBlocks.begin(),
                          alwaysBlocks.end());

  for (SL::NodeId const block : proceduralBlocks) {
    CheckExpressionsIteratively(fileContent, block, errors, symbols);
  }
}