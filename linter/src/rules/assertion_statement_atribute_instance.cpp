#include "rules/assertion_statement_atribute_instance.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <algorithm>
#include <array>
#include <string_view>

#include "main/lint_rules.h"
#include "utils/ast_utils.h"
#include "utils/location_utils.h"

namespace SL = SURELOG;

static constexpr std::array kValidAfterLabelTypes = {
    SL::VObjectType::paStatement_item,
    SL::VObjectType::paAttribute_instance,
};

namespace {
auto CheckModuleLevelCover(const SL::FileContent* fileContent,
                           SL::NodeId coverPropertyStmt) {
  auto assertionItem =
      FindAncestorOfType(fileContent, coverPropertyStmt,
                         SL::VObjectType::paConcurrent_assertion_item);
  if (!assertionItem) {
    return false;
  }

  SL::NodeId const firstChild = fileContent->Child(assertionItem);
  if (!firstChild ||
      fileContent->Type(firstChild) != SL::VObjectType::slStringConst) {
    return false;
  }

  auto moduleOrGenItem = FindAncestorOfType(
      fileContent, assertionItem, SL::VObjectType::paModule_or_generate_item);
  if (!moduleOrGenItem) {
    return false;
  }

  SL::NodeId const firstItemChild = fileContent->Child(moduleOrGenItem);
  if (!firstItemChild) {
    return true;
  }

  return (fileContent->Type(firstItemChild) !=
          SL::VObjectType::paAttribute_instance);
}

auto CheckProceduralCover(const SL::FileContent* fileContent,
                          SL::NodeId coverPropertyStmt) {
  SL::NodeId const proceduralAssert =
      FindAncestorOfType(fileContent, coverPropertyStmt,
                         SL::VObjectType::paProcedural_assertion_statement);
  if (!proceduralAssert) {
    return false;
  }

  auto stmt = FindAncestorOfType(fileContent, proceduralAssert,
                                 SL::VObjectType::paStatement);
  if (!stmt) {
    return false;
  }

  SL::NodeId const firstChild = fileContent->Child(stmt);
  if (!firstChild ||
      fileContent->Type(firstChild) != SL::VObjectType::slStringConst) {
    return false;
  }

  SL::NodeId const afterLabel = fileContent->Sibling(firstChild);
  if (!afterLabel) {
    {
      return true;
    }
  }

  SL::VObjectType const afterLabelType = fileContent->Type(afterLabel);
  return std::ranges::any_of(kValidAfterLabelTypes,
                             [afterLabelType](SL::VObjectType type) {
                               return type == afterLabelType;
                             });
}

auto ExtractLabelName(const SL::FileContent* fileContent,
                      SL::NodeId coverPropertyStmt) {
  auto getFirstStringConst = [&](SL::NodeId node) -> std::string_view {
    SL::NodeId const child = fileContent->Child(node);
    if (child && fileContent->Type(child) == SL::VObjectType::slStringConst) {
      return fileContent->SymName(child);
    }
    return {};
  };

  auto assertionItem =
      FindAncestorOfType(fileContent, coverPropertyStmt,
                         SL::VObjectType::paConcurrent_assertion_item);
  if (assertionItem) {
    if (auto name = getFirstStringConst(assertionItem); !name.empty()) {
      return name;
    }
  }

  return static_cast<std::string_view>("<unknown>");
}
}  // namespace

void CheckAssertionStatementAttributeInstance(
    const SL::FileContent* fileContent, SL::ErrorContainer* errors,
    SL::SymbolTable* symbols) {
  if (fileContent == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }

  SL::NodeId const root = fileContent->getRootNode();
  if (!root) {
    return;
  }

  for (SL::NodeId const coverStmt : fileContent->sl_collect_all(
           root, SL::VObjectType::paCover_property_statement)) {
    const bool kIsProcedural =
        FindAncestorOfType(fileContent, coverStmt,
                           SL::VObjectType::paProcedural_assertion_statement) !=
        SL::InvalidNodeId;

    const bool kHasViolation =
        kIsProcedural ? CheckProceduralCover(fileContent, coverStmt)
                      : CheckModuleLevelCover(fileContent, coverStmt);

    if (!kHasViolation) {
      continue;
    }

    ReportError(fileContent, coverStmt,
                ExtractLabelName(fileContent, coverStmt),
                verihogg_lint::LINT_ASSERTION_STATEMENT_ATTRIBUTE_INSTANCE,
                errors, symbols);
  }
}