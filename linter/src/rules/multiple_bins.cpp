#include "rules/multiple_bins.h"

#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <algorithm>
#include <array>
#include <stack>
#include <string_view>

#include "utils/location_utils.h"
#include "utils/name_utils.h"

namespace SL = SURELOG;

static constexpr std::array kValueTypes = {
    SL::VObjectType::slIntConst,
    SL::VObjectType::slStringConst,
};

static auto ValueHasWildcard(std::string_view val) -> bool {
  static constexpr std::string_view kWildcardChars = "xXzZ?";
  return std::ranges::any_of(val, [](char chr) {
    return kWildcardChars.find(chr) != std::string_view::npos;
  });
}

static auto FindWildcardInTransRangeList(const SL::FileContent* fileContent,
                                         SL::NodeId node) -> SL::NodeId {
  if (node == SL::InvalidNodeId) {
    return SL::InvalidNodeId;
  }

  std::stack<SL::NodeId> stack;
  stack.push(node);

  while (!stack.empty()) {
    SL::NodeId current = stack.top();
    stack.pop();

    SL::VObjectType type = fileContent->Type(current);

    if (type == SL::VObjectType::paNumber_1Tickbx) {
      return current;
    }

    if (std::ranges::any_of(
            kValueTypes, [type](SL::VObjectType tpe) { return tpe == type; })) {
      if (ValueHasWildcard(fileContent->SymName(current))) {
        return current;
      }
    }

    std::stack<SL::NodeId> children;
    for (SL::NodeId child = fileContent->Child(current); child;
         child = fileContent->Sibling(child)) {
      children.push(child);
    }
    while (!children.empty()) {
      stack.push(children.top());
      children.pop();
    }
  }

  return SL::InvalidNodeId;
}

static auto FindWildcardInTransList(const SL::FileContent* fileContent,
                                    SL::NodeId transList) -> SL::NodeId {
  if (transList == SL::InvalidNodeId) {
    return SL::InvalidNodeId;
  }

  for (SL::NodeId transSet = fileContent->Child(transList); transSet;
       transSet = fileContent->Sibling(transSet)) {
    if (fileContent->Type(transSet) != SL::VObjectType::paTrans_set) {
      continue;
    }

    for (SL::NodeId transRange = fileContent->Child(transSet); transRange;
         transRange = fileContent->Sibling(transRange)) {
      if (fileContent->Type(transRange) !=
          SL::VObjectType::paTrans_range_list) {
        continue;
      }

      SL::NodeId wildcardNode =
          FindWildcardInTransRangeList(fileContent, transRange);
      if (wildcardNode) {
        return wildcardNode;
      }
    }
  }

  return SL::InvalidNodeId;
}

static auto ExtractBinName(const SL::FileContent* fileContent,
                           SL::NodeId binsOrOptions) -> std::string_view {
  if (fileContent == nullptr || !binsOrOptions) {
    return "<unknown>";
  }

  for (SL::NodeId child = fileContent->Child(binsOrOptions); child;
       child = fileContent->Sibling(child)) {
    if (fileContent->Type(child) == SL::VObjectType::paBins_Bins) {
      SL::NodeId nameNode = fileContent->Sibling(child);
      if (nameNode &&
          fileContent->Type(nameNode) == SL::VObjectType::slStringConst) {
        return fileContent->SymName(nameNode);
      }
    }
  }

  return "<unknown>";
}

static auto FindTransListInBinsOrOptions(const SL::FileContent* fileContent,
                                         SL::NodeId binsOrOptions)
    -> SL::NodeId {
  if (!binsOrOptions) {
    return SL::InvalidNodeId;
  }
  auto transLists =
      fileContent->sl_collect_all(binsOrOptions, SL::VObjectType::paTrans_list);
  return transLists.empty() ? SL::InvalidNodeId : transLists.front();
}

void CheckMultipleBins(const SL::FileContent* fileContent,
                       SL::ErrorContainer* errors, SL::SymbolTable* symbols) {
  if (fileContent == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }

  SL::NodeId root = fileContent->getRootNode();
  if (root == SL::InvalidNodeId) {
    return;
  }

  for (SL::NodeId binsNode :
       fileContent->sl_collect_all(root, SL::VObjectType::paBins_or_options)) {
    SL::NodeId transList = FindTransListInBinsOrOptions(fileContent, binsNode);
    if (transList == SL::InvalidNodeId) {
      continue;
    }

    SL::NodeId wildcardNode = FindWildcardInTransList(fileContent, transList);
    if (wildcardNode == SL::InvalidNodeId) {
      continue;
    }

    ReportError(fileContent, wildcardNode,
                ExtractBinName(fileContent, binsNode),
                SL::ErrorDefinition::LINT_MULTIPLE_BINS, errors, symbols);
  }
}