#include "rules/fork_disable.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <string>
#include <unordered_set>
#include <vector>

#include "main/lint_rules.h"
#include "utils/location_utils.h"

namespace {

namespace SL = SURELOG;

constexpr SL::VObjectType kPaParBlock = SL::VObjectType::paPar_block;
constexpr SL::VObjectType kPaDisableStatement =
    SL::VObjectType::paDisable_statement;
constexpr SL::VObjectType kStringConst = SL::VObjectType::slStringConst;

struct ForkInfo {
  std::string name;
  std::unordered_set<unsigned int> descendants;
};

void CollectDescendants(const SL::FileContent* fc, SL::NodeId node,
                        std::unordered_set<unsigned int>& out) {
  if (node == SL::InvalidNodeId) {
    return;
  }
  out.insert(static_cast<unsigned int>(node));
  for (auto c = fc->Child(node); c != SL::InvalidNodeId; c = fc->Sibling(c)) {
    CollectDescendants(fc, c, out);
  }
}

SL::NodeId FindStringConst(const SL::FileContent* fc, SL::NodeId node) {
  if (node == SL::InvalidNodeId) {
    return SL::InvalidNodeId;
  }
  if (fc->Type(node) == kStringConst) {
    return node;
  }
  for (auto c = fc->Child(node); c != SL::InvalidNodeId; c = fc->Sibling(c)) {
    auto r = FindStringConst(fc, c);
    if (r != SL::InvalidNodeId) {
      return r;
    }
  }
  return SL::InvalidNodeId;
}

bool IsInAnyForkScope(const std::vector<ForkInfo>& forks,
                      const std::string& targetName, SL::NodeId disNode) {
  for (const auto& f : forks) {
    if (f.name != targetName) {
      continue;
    }
    if (f.descendants.count(static_cast<unsigned int>(disNode)) != 0) {
      return true;
    }
  }
  return false;
}

bool IsForkName(const std::vector<ForkInfo>& forks, const std::string& name) {
  for (const auto& f : forks) {
    if (f.name == name) {
      return true;
    }
  }
  return false;
}

}  // namespace

void CheckForkDisable(const SURELOG::FileContent* fileContent,
                      SURELOG::ErrorContainer* errors,
                      SURELOG::SymbolTable* symbols) {
  if (fileContent == nullptr) {
    return;
  }

  std::vector<ForkInfo> forks;
  for (auto pb :
       fileContent->sl_collect_all(fileContent->getRootNode(), kPaParBlock)) {
    auto first = fileContent->Child(pb);
    if (first == SL::InvalidNodeId) {
      continue;
    }
    if (fileContent->Type(first) != kStringConst) {
      continue;
    }
    ForkInfo info;
    info.name = std::string(fileContent->SymName(first));
    CollectDescendants(fileContent, pb, info.descendants);
    forks.push_back(std::move(info));
  }

  if (forks.empty()) {
    return;
  }

  for (auto dis : fileContent->sl_collect_all(fileContent->getRootNode(),
                                              kPaDisableStatement)) {
    const SL::NodeId target = FindStringConst(fileContent, dis);
    if (target == SL::InvalidNodeId) {
      continue;
    }
    const std::string targetName(fileContent->SymName(target));

    if (!IsForkName(forks, targetName)) {
      continue;
    }

    if (IsInAnyForkScope(forks, targetName, dis)) {
      continue;
    }

    ReportError(fileContent, target, targetName,
                verihogg_lint::LINT_FORK_DISABLE, errors, symbols);
  }
}
