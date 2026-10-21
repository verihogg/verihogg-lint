#include "rules/fork_variable_access.h"

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

constexpr SL::VObjectType kConstKw = SL::VObjectType::paCONST;
constexpr SL::VObjectType kConstTypeKw = SL::VObjectType::paConst_type;
constexpr SL::VObjectType kRefKw = SL::VObjectType::paREF;
constexpr SL::VObjectType kInoutKw = SL::VObjectType::paINOUT;
constexpr SL::VObjectType kDataDecl = SL::VObjectType::paData_declaration;
constexpr SL::VObjectType kForInit = SL::VObjectType::paFor_initialization;
constexpr SL::VObjectType kForVarDecl =
    SL::VObjectType::paFor_variable_declaration;
constexpr SL::VObjectType kTfPortItem = SL::VObjectType::paTf_port_item;
constexpr SL::VObjectType kDollarKw = SL::VObjectType::paDollar_keyword;

bool IsJoinKeyword(SL::VObjectType t) {
  return t == SL::VObjectType::paJoin_keyword ||
         t == SL::VObjectType::paJoin_any_keyword ||
         t == SL::VObjectType::paJoin_none_keyword;
}

bool ContainsType(const SL::FileContent* fc, SL::NodeId node,
                  SL::VObjectType type) {
  if (node == SL::InvalidNodeId) {
    return false;
  }
  if (fc->Type(node) == type) {
    return true;
  }
  for (auto c = fc->Child(node); c != SL::InvalidNodeId; c = fc->Sibling(c)) {
    if (ContainsType(fc, c, type)) {
      return true;
    }
  }
  return false;
}

bool HasConstQualifier(const SL::FileContent* fc, SL::NodeId node) {
  return ContainsType(fc, node, kConstKw) ||
         ContainsType(fc, node, kConstTypeKw);
}

std::string NameOf(const SL::FileContent* fc, SL::NodeId node) {
  return std::string(fc->SymName(node));
}

void CollectStringsOutsideNestedForks(const SL::FileContent* fc,
                                      SL::NodeId node,
                                      std::vector<SL::NodeId>& out) {
  if (node == SL::InvalidNodeId) {
    return;
  }
  if (fc->Type(node) == SL::VObjectType::paPar_block) {
    return;
  }
  if (fc->Type(node) == SL::VObjectType::slStringConst) {
    out.push_back(node);
    return;
  }
  for (auto c = fc->Child(node); c != SL::InvalidNodeId; c = fc->Sibling(c)) {
    CollectStringsOutsideNestedForks(fc, c, out);
  }
}

void CollectStringsForAccessCheck(const SL::FileContent* fc, SL::NodeId node,
                                  std::vector<SL::NodeId>& out,
                                  SL::NodeId parent = SL::InvalidNodeId) {
  if (node == SL::InvalidNodeId) {
    return;
  }
  const auto t = fc->Type(node);
  if (t == SL::VObjectType::paPar_block) {
    return;
  }
  if (t == SL::VObjectType::paBlock_item_declaration) {
    return;
  }
  {
    SL::NodeId prevStr = SL::InvalidNodeId;
    SL::NodeId lastStr = SL::InvalidNodeId;
    bool hasAdjacent = false;
    for (auto c = fc->Child(node); c != SL::InvalidNodeId; c = fc->Sibling(c)) {
      if (fc->Type(c) != SL::VObjectType::slStringConst) {
        continue;
      }
      if (prevStr != SL::InvalidNodeId && fc->Sibling(prevStr) == c) {
        hasAdjacent = true;
      }
      prevStr = c;
      lastStr = c;
    }
    if (hasAdjacent) {
      out.push_back(lastStr);
      return;
    }
  }
  if (t == SL::VObjectType::paPs_or_hierarchical_identifier) {
    if (parent != SL::InvalidNodeId) {
      for (auto s = fc->Sibling(node); s != SL::InvalidNodeId;
           s = fc->Sibling(s)) {
        if (ContainsType(fc, s, SL::VObjectType::slStringConst)) {
          return;
        }
      }
    }
    SL::NodeId lastStr = SL::InvalidNodeId;
    for (auto c = fc->Child(node); c != SL::InvalidNodeId; c = fc->Sibling(c)) {
      if (fc->Type(c) == SL::VObjectType::slStringConst) {
        lastStr = c;
      }
    }
    if (lastStr != SL::InvalidNodeId) {
      out.push_back(lastStr);
      return;
    }
    for (auto c = fc->Child(node); c != SL::InvalidNodeId; c = fc->Sibling(c)) {
      CollectStringsForAccessCheck(fc, c, out, node);
    }
    return;
  }
  if (t == SL::VObjectType::paSubroutine_call) {
    for (auto c = fc->Child(node); c != SL::InvalidNodeId; c = fc->Sibling(c)) {
      if (fc->Type(c) == SL::VObjectType::slStringConst) {
        continue;
      }
      CollectStringsForAccessCheck(fc, c, out, node);
    }
    return;
  }
  if (t == SL::VObjectType::slStringConst) {
    if (parent != SL::InvalidNodeId) {
      for (auto sib = fc->Child(parent); sib != SL::InvalidNodeId;
           sib = fc->Sibling(sib)) {
        if (sib == node) {
          break;
        }
        if (fc->Type(sib) == kDollarKw) {
          return;
        }
      }
    }
    out.push_back(node);
    return;
  }
  for (auto c = fc->Child(node); c != SL::InvalidNodeId; c = fc->Sibling(c)) {
    CollectStringsForAccessCheck(fc, c, out, node);
  }
}

void CollectBlockItemDecls(const SL::FileContent* fc, SL::NodeId node,
                           std::vector<SL::NodeId>& out) {
  if (node == SL::InvalidNodeId) {
    return;
  }
  const auto t = fc->Type(node);
  if (t == SL::VObjectType::paPar_block) {
    return;
  }
  if (t == SL::VObjectType::paBlock_item_declaration) {
    out.push_back(node);
    return;
  }
  for (auto c = fc->Child(node); c != SL::InvalidNodeId; c = fc->Sibling(c)) {
    CollectBlockItemDecls(fc, c, out);
  }
}

void CollectForInitNames(const SL::FileContent* fc, SL::NodeId node,
                         std::unordered_set<std::string>& out) {
  if (node == SL::InvalidNodeId) {
    return;
  }
  if (fc->Type(node) == SL::VObjectType::paPar_block) {
    return;
  }
  if (fc->Type(node) == kForInit || fc->Type(node) == kForVarDecl) {
    std::vector<SL::NodeId> strings;
    CollectStringsOutsideNestedForks(fc, node, strings);
    for (auto s : strings) {
      out.insert(NameOf(fc, s));
    }
    return;
  }
  for (auto c = fc->Child(node); c != SL::InvalidNodeId; c = fc->Sibling(c)) {
    CollectForInitNames(fc, c, out);
  }
}

std::unordered_set<std::string> CollectConstantNames(
    const SL::FileContent* fc) {
  std::unordered_set<std::string> names;
  auto root = fc->getRootNode();

  auto collectFromDecl = [&](SL::VObjectType t) {
    for (auto d : fc->sl_collect_all(root, t)) {
      std::vector<SL::NodeId> strings;
      CollectStringsOutsideNestedForks(fc, d, strings);
      for (auto s : strings) {
        names.insert(NameOf(fc, s));
      }
    }
  };

  collectFromDecl(SL::VObjectType::paParameter_declaration);
  collectFromDecl(SL::VObjectType::paLocal_parameter_declaration);
  collectFromDecl(SL::VObjectType::paEnum_name_declaration);
  collectFromDecl(SL::VObjectType::paGenvar_declaration);

  for (auto d : fc->sl_collect_all(root, kDataDecl)) {
    if (!HasConstQualifier(fc, d)) {
      continue;
    }
    std::vector<SL::NodeId> strings;
    CollectStringsOutsideNestedForks(fc, d, strings);
    for (auto s : strings) {
      names.insert(NameOf(fc, s));
    }
  }

  return names;
}

struct RefArgs {
  std::unordered_set<std::string> allRef;
  std::unordered_set<std::string> constRef;
};

RefArgs CollectRefArgs(const SL::FileContent* fc) {
  RefArgs r;
  auto root = fc->getRootNode();

  for (auto p : fc->sl_collect_all(root, kTfPortItem)) {
    const bool isRef =
        ContainsType(fc, p, kRefKw) || ContainsType(fc, p, kInoutKw);
    if (!isRef) {
      continue;
    }
    const bool isConst = HasConstQualifier(fc, p);

    std::vector<SL::NodeId> strings;
    CollectStringsOutsideNestedForks(fc, p, strings);

    for (auto s : strings) {
      const std::string name = NameOf(fc, s);
      r.allRef.insert(name);
      if (isConst) {
        r.constRef.insert(name);
      }
    }
  }
  return r;
}

bool LooksLikeLiteral(const std::string& s) {
  if (s.empty()) {
    return true;
  }
  const char c = s.front();
  return c == '"' || c == '\'' || c == '$';
}

}  // namespace

void CheckForkVariableAccess(const SURELOG::FileContent* fileContent,
                             SURELOG::ErrorContainer* errors,
                             SURELOG::SymbolTable* symbols) {
  if (fileContent == nullptr) {
    return;
  }

  const auto constantNames = CollectConstantNames(fileContent);
  const auto refArgs = CollectRefArgs(fileContent);
  const auto forks = fileContent->sl_collect_all(fileContent->getRootNode(),
                                                 SL::VObjectType::paPar_block);

  for (auto fork : forks) {
    std::unordered_set<unsigned int> skipNodes;

    auto first = fileContent->Child(fork);
    if (first != SL::InvalidNodeId &&
        fileContent->Type(first) == SL::VObjectType::slStringConst) {
      skipNodes.insert(static_cast<unsigned int>(first));
    }

    for (auto c = fileContent->Child(fork); c != SL::InvalidNodeId;
         c = fileContent->Sibling(c)) {
      if (IsJoinKeyword(fileContent->Type(c))) {
        auto sib = fileContent->Sibling(c);
        if (sib != SL::InvalidNodeId &&
            fileContent->Type(sib) == SL::VObjectType::slStringConst) {
          skipNodes.insert(static_cast<unsigned int>(sib));
        }
        break;
      }
    }

    std::unordered_set<std::string> localNames;
    std::vector<SL::NodeId> decls;
    for (auto c = fileContent->Child(fork); c != SL::InvalidNodeId;
         c = fileContent->Sibling(c)) {
      CollectBlockItemDecls(fileContent, c, decls);
      CollectForInitNames(fileContent, c, localNames);
    }
    for (auto d : decls) {
      std::vector<SL::NodeId> strings;
      CollectStringsOutsideNestedForks(fileContent, d, strings);
      for (auto s : strings) {
        localNames.insert(NameOf(fileContent, s));
      }
    }

    std::vector<SL::NodeId> usages;
    for (auto c = fileContent->Child(fork); c != SL::InvalidNodeId;
         c = fileContent->Sibling(c)) {
      CollectStringsForAccessCheck(fileContent, c, usages);
    }

    for (auto s : usages) {
      if (skipNodes.contains(static_cast<unsigned int>(s))) {
        continue;
      }
      const std::string name = NameOf(fileContent, s);
      if (LooksLikeLiteral(name)) {
        continue;
      }
      if (localNames.count(name)) {
        continue;
      }
      if (constantNames.count(name)) {
        continue;
      }
      if (refArgs.constRef.count(name)) {
        continue;
      }

      ReportError(fileContent, s, name,
                  verihogg_lint::LINT_FORK_VARIABLE_ACCESS, errors, symbols);
    }
  }
}
