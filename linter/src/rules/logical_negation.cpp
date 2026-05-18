#include "rules/logical_negation.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

#include "main/lint_rules.h"
#include "utils/ast_utils.h"
#include "utils/location_utils.h"

namespace SL = SURELOG;

namespace {

auto ResolveSimpleVarTypeName(const SL::FileContent* fc,
                              SL::NodeId primaryLiteral,
                              const std::unordered_set<std::string>& classSet)
    -> std::string_view {
  SL::NodeId const kNameNode = fc->Child(primaryLiteral);
  if (!kNameNode || fc->Type(kNameNode) != SL::VObjectType::slStringConst) {
    return {};
  }

  std::string_view const kVarName = fc->SymName(kNameNode);
  SL::NodeId const kDecl = FindVarOrNetDecl(fc, fc->getRootNode(), kVarName);
  if (!kDecl) {
    return {};
  }

  std::string_view const kTypeName = GetTypedefName(fc, kDecl);
  if (kTypeName.empty() || !classSet.count(std::string(kTypeName))) {
    return {};
  }
  return kTypeName;
}

auto ResolveMemberAccessTypeName(
    const SL::FileContent* fc, SL::NodeId complexFuncCall,
    const std::unordered_set<std::string>& classSet) -> std::string_view {
  std::vector<std::string_view> names;
  for (SL::NodeId cur = fc->Child(complexFuncCall); cur;
       cur = fc->Sibling(cur)) {
    if (fc->Type(cur) == SL::VObjectType::slStringConst) {
      names.push_back(fc->SymName(cur));
    }
  }
  if (names.size() < 2) {
    return {};
  }

  SL::NodeId const kRootDecl =
      FindVarOrNetDecl(fc, fc->getRootNode(), names.at(0));
  if (!kRootDecl) {
    return {};
  }

  std::string_view currentTypeName = GetTypedefName(fc, kRootDecl);
  if (currentTypeName.empty() ||
      !classSet.count(std::string(currentTypeName))) {
    return {};
  }

  for (std::size_t i = 1; i < names.size(); ++i) {
    SL::NodeId const kClassDecl = GetClassDeclByName(fc, currentTypeName);
    if (!kClassDecl) {
      return {};
    }

    SL::NodeId const kMemberDecl =
        FindVarOrNetDecl(fc, kClassDecl, names.at(i));
    if (!kMemberDecl) {
      return {};
    }

    currentTypeName = GetTypedefName(fc, kMemberDecl);
    if (currentTypeName.empty() ||
        !classSet.count(std::string(currentTypeName))) {
      return {};
    }
  }

  return currentTypeName;
}

auto GetOperandClassTypeName(const SL::FileContent* fc, SL::NodeId unaryNotNode,
                             const std::unordered_set<std::string>& classSet)
    -> std::string_view {
  SL::NodeId const kOperandExpr = fc->Sibling(unaryNotNode);
  if (!kOperandExpr ||
      fc->Type(kOperandExpr) != SL::VObjectType::paExpression) {
    return {};
  }

  SL::NodeId const kPrimary = fc->Child(kOperandExpr);
  if (!kPrimary || fc->Type(kPrimary) != SL::VObjectType::paPrimary) {
    return {};
  }

  SL::NodeId const kPrimaryChild = fc->Child(kPrimary);
  if (!kPrimaryChild) {
    return {};
  }

  switch (fc->Type(kPrimaryChild)) {
    case SL::VObjectType::paPrimary_literal:
      return ResolveSimpleVarTypeName(fc, kPrimaryChild, classSet);

    case SL::VObjectType::paComplex_func_call:
      return ResolveMemberAccessTypeName(fc, kPrimaryChild, classSet);

    default:
      return {};
  }
}

}  // namespace

void CheckLogicalNegation(const SL::FileContent* fileContent,
                          SL::ErrorContainer* errors,
                          SL::SymbolTable* symbols) {
  if (fileContent == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }

  SL::NodeId const kRoot = fileContent->getRootNode();
  if (!kRoot) {
    return;
  }

  auto const kClassSet = GetClassSetFromFileContent(fileContent);
  if (kClassSet.empty()) {
    return;
  }

  for (SL::NodeId const kUnaryNot :
       fileContent->sl_collect_all(kRoot, SL::VObjectType::paUnary_Not)) {
    std::string_view const kTypeName =
        GetOperandClassTypeName(fileContent, kUnaryNot, kClassSet);
    if (!kTypeName.empty()) {
      ReportError(fileContent, kUnaryNot, kTypeName,
                  verihogg_lint::LINT_LOGICAL_NEGATION, errors, symbols);
    }
  }
}
