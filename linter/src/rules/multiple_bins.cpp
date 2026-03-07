#include "rules/multiple_bins.h"

#include <algorithm>
#include <array>
#include <string_view>

#include "Surelog/Design/FileContent.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"
#include "Surelog/SourceCompile/VObjectTypes.h"
#include "utils/location_utils.h"
#include "utils/name_utils.h"

using namespace SURELOG;

static constexpr std::array kValueTypes = {
    VObjectType::slIntConst,
    VObjectType::slStringConst,
};

static bool valueHasWildcard(std::string_view val) {
  static constexpr std::string_view kWildcardChars = "xXzZ?";
  return std::ranges::any_of(val, [](char c) {
    return kWildcardChars.find(c) != std::string_view::npos;
  });
}

static NodeId findWildcardInTransRangeList(const FileContent* fC, NodeId node) {
  if (!node) return InvalidNodeId;

  VObjectType type = fC->Type(node);

  if (type == VObjectType::paNumber_1Tickbx) {
    return node;
  }

  if (std::ranges::any_of(kValueTypes,
                          [type](VObjectType t) { return t == type; })) {
    if (valueHasWildcard(fC->SymName(node))) return node;
  }

  for (NodeId child = fC->Child(node); child; child = fC->Sibling(child)) {
    NodeId found = findWildcardInTransRangeList(fC, child);
    if (found) return found;
  }

  return InvalidNodeId;
}

static NodeId findWildcardInTransList(const FileContent* fC, NodeId transList) {
  if (!transList) return InvalidNodeId;

  for (NodeId transSet = fC->Child(transList); transSet;
       transSet = fC->Sibling(transSet)) {
    if (fC->Type(transSet) != VObjectType::paTrans_set) continue;

    for (NodeId transRange = fC->Child(transSet); transRange;
         transRange = fC->Sibling(transRange)) {
      if (fC->Type(transRange) != VObjectType::paTrans_range_list) continue;

      NodeId wildcardNode = findWildcardInTransRangeList(fC, transRange);
      if (wildcardNode) return wildcardNode;
    }
  }

  return InvalidNodeId;
}

static std::string_view extractBinName(const FileContent* fC,
                                       NodeId binsOrOptions) {
  if (!fC || !binsOrOptions) return "<unknown>";

  for (NodeId child = fC->Child(binsOrOptions); child;
       child = fC->Sibling(child)) {
    if (fC->Type(child) == VObjectType::paBins_Bins) {
      NodeId nameNode = fC->Sibling(child);
      if (nameNode && fC->Type(nameNode) == VObjectType::slStringConst) {
        return fC->SymName(nameNode);
      }
    }
  }

  return "<unknown>";
}

static NodeId findTransListInBinsOrOptions(const FileContent* fC,
                                           NodeId binsOrOptions) {
  if (!binsOrOptions) return InvalidNodeId;
  auto transLists =
      fC->sl_collect_all(binsOrOptions, VObjectType::paTrans_list);
  return transLists.empty() ? InvalidNodeId : transLists.front();
}

void checkMultipleBins(const FileContent* fC, ErrorContainer* errors,
                       SymbolTable* symbols) {
  if (!fC || !errors || !symbols) return;

  NodeId root = fC->getRootNode();
  if (!root) return;

  for (NodeId binsNode :
       fC->sl_collect_all(root, VObjectType::paBins_or_options)) {
    NodeId transList = findTransListInBinsOrOptions(fC, binsNode);
    if (!transList) continue;

    NodeId wildcardNode = findWildcardInTransList(fC, transList);
    if (!wildcardNode) continue;

    reportError(fC, wildcardNode, extractBinName(fC, binsNode),
                ErrorDefinition::LINT_MULTIPLE_BINS, errors, symbols);
  }
}