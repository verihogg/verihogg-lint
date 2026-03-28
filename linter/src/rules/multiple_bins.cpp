#include "rules/multiple_bins.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <algorithm>
#include <array>
#include <stack>
#include <string_view>

#include "main/lint_rules.h"
#include "utils/location_utils.h"

namespace SL = SURELOG;

static constexpr std::array kValueTypes = {
    SL::VObjectType::slIntConst,
    SL::VObjectType::slStringConst,
};

namespace {
auto ValueHasWildcard(std::string_view val) -> bool {
  static constexpr std::string_view kWildcardChars = "xXzZ?";
  return std::ranges::any_of(val, [](char chr) -> bool {
    return kWildcardChars.find(chr) != std::string_view::npos;
  });
}

auto FindWildcardInTransRangeList(const SL::FileContent* fileContent,
                                  SL::NodeId node) -> SL::NodeId {
  if (node == SL::InvalidNodeId) {
    return SL::InvalidNodeId;
  }

  std::stack<SL::NodeId> stack;
  stack.push(node);

  while (!stack.empty()) {
    SL::NodeId const kCurrent = stack.top();
    stack.pop();

    SL::VObjectType const kType = fileContent->Type(kCurrent);

    if (kType == SL::VObjectType::paNumber_1Tickbx) {
      return kCurrent;
    }

    if (std::ranges::any_of(kValueTypes, [kType](SL::VObjectType tpe) -> bool {
          return tpe == kType;
        })) {
      if (ValueHasWildcard(fileContent->SymName(kCurrent))) {
        return kCurrent;
      }
    }

    std::stack<SL::NodeId> children;
    for (SL::NodeId child = fileContent->Child(kCurrent); child;
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

auto FindWildcardInTransList(const SL::FileContent* fileContent,
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

      SL::NodeId const kWildcardNode =
          FindWildcardInTransRangeList(fileContent, transRange);
      if (kWildcardNode) {
        return kWildcardNode;
      }
    }
  }

  return SL::InvalidNodeId;
}

auto ExtractBinName(const SL::FileContent* fileContent,
                    SL::NodeId binsOrOptions) -> std::string_view {
  if (fileContent == nullptr || !binsOrOptions) {
    return "<unknown>";
  }

  for (SL::NodeId child = fileContent->Child(binsOrOptions); child;
       child = fileContent->Sibling(child)) {
    if (fileContent->Type(child) == SL::VObjectType::paBins_Bins) {
      SL::NodeId const kNameNode = fileContent->Sibling(child);
      if (kNameNode &&
          fileContent->Type(kNameNode) == SL::VObjectType::slStringConst) {
        return fileContent->SymName(kNameNode);
      }
    }
  }

  return "<unknown>";
}

auto FindTransListInBinsOrOptions(const SL::FileContent* fileContent,
                                  SL::NodeId binsOrOptions) -> SL::NodeId {
  if (!binsOrOptions) {
    return SL::InvalidNodeId;
  }
  auto transLists =
      fileContent->sl_collect_all(binsOrOptions, SL::VObjectType::paTrans_list);
  return transLists.empty() ? SL::InvalidNodeId : transLists.front();
}
}  // namespace

void CheckMultipleBins(const SL::FileContent* fileContent,
                       SL::ErrorContainer* errors, SL::SymbolTable* symbols) {
  if (fileContent == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }

  SL::NodeId const kRoot = fileContent->getRootNode();
  if (kRoot == SL::InvalidNodeId) {
    return;
  }

  for (SL::NodeId const kBinsNode :
       fileContent->sl_collect_all(kRoot, SL::VObjectType::paBins_or_options)) {
    SL::NodeId const kTransList =
        FindTransListInBinsOrOptions(fileContent, kBinsNode);
    if (kTransList == SL::InvalidNodeId) {
      continue;
    }

    SL::NodeId const kWildcardNode =
        FindWildcardInTransList(fileContent, kTransList);
    if (kWildcardNode == SL::InvalidNodeId) {
      continue;
    }

    ReportError(fileContent, kWildcardNode,
                ExtractBinName(fileContent, kBinsNode),
                verihogg_lint::LINT_MULTIPLE_BINS, errors, symbols);
  }
}