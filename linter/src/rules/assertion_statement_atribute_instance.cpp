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

  SL::NodeId const kFirstChild = fileContent->Child(assertionItem);
  if (!kFirstChild ||
      fileContent->Type(kFirstChild) != SL::VObjectType::slStringConst) {
    return false;
  }

  auto moduleOrGenItem = FindAncestorOfType(
      fileContent, assertionItem, SL::VObjectType::paModule_or_generate_item);
  if (!moduleOrGenItem) {
    return false;
  }

  SL::NodeId const kFirstItemChild = fileContent->Child(moduleOrGenItem);
  if (!kFirstItemChild) {
    return true;
  }

  return (fileContent->Type(kFirstItemChild) !=
          SL::VObjectType::paAttribute_instance);
}

auto CheckProceduralCover(const SL::FileContent* fileContent,
                          SL::NodeId coverPropertyStmt) {
  SL::NodeId const kProceduralAssert =
      FindAncestorOfType(fileContent, coverPropertyStmt,
                         SL::VObjectType::paProcedural_assertion_statement);
  if (!kProceduralAssert) {
    return false;
  }

  auto stmt = FindAncestorOfType(fileContent, kProceduralAssert,
                                 SL::VObjectType::paStatement);
  if (!stmt) {
    return false;
  }

  SL::NodeId const kFirstChild = fileContent->Child(stmt);
  if (!kFirstChild ||
      fileContent->Type(kFirstChild) != SL::VObjectType::slStringConst) {
    return false;
  }

  SL::NodeId const kAfterLabel = fileContent->Sibling(kFirstChild);
  if (!kAfterLabel) {
    {
      return true;
    }
  }

  SL::VObjectType const kAfterLabelType = fileContent->Type(kAfterLabel);
  return std::ranges::any_of(kValidAfterLabelTypes,
                             [kAfterLabelType](SL::VObjectType type) {
                               return type == kAfterLabelType;
                             });
}

auto ExtractLabelName(const SL::FileContent* fileContent,
                      SL::NodeId coverPropertyStmt) {
  auto getFirstStringConst = [&](SL::NodeId node) -> std::string_view {
    SL::NodeId const kChild = fileContent->Child(node);
    if (kChild && fileContent->Type(kChild) == SL::VObjectType::slStringConst) {
      return fileContent->SymName(kChild);
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

  SL::NodeId const kRoot = fileContent->getRootNode();
  if (!kRoot) {
    return;
  }

  for (SL::NodeId const kCoverStmt : fileContent->sl_collect_all(
           kRoot, SL::VObjectType::paCover_property_statement)) {
    const bool kIsProcedural =
        FindAncestorOfType(fileContent, kCoverStmt,
                           SL::VObjectType::paProcedural_assertion_statement) !=
        SL::InvalidNodeId;

    const bool kHasViolation =
        kIsProcedural ? CheckProceduralCover(fileContent, kCoverStmt)
                      : CheckModuleLevelCover(fileContent, kCoverStmt);

    if (!kHasViolation) {
      continue;
    }

    ReportError(fileContent, kCoverStmt,
                ExtractLabelName(fileContent, kCoverStmt),
                verihogg_lint::LINT_ASSERTION_STATEMENT_ATTRIBUTE_INSTANCE,
                errors, symbols);
  }
}