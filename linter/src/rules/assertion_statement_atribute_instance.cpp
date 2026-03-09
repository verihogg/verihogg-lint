#include "rules/assertion_statement_atribute_instance.h"

#include <algorithm>
#include <array>
#include <string_view>

#include "Surelog/Design/FileContent.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"
#include "Surelog/SourceCompile/VObjectTypes.h"
#include "utils/ast_utils.h"
#include "utils/location_utils.h"
#include "utils/name_utils.h"

using namespace SURELOG;

static constexpr std::array kValidAfterLabelTypes = {
    VObjectType::paStatement_item,
    VObjectType::paAttribute_instance,
};

static bool CheckModuleLevelCover(const FileContent* fC,
                                  NodeId coverPropertyStmt) {
  NodeId assertionItem = FindAncestorOfType(
      fC, coverPropertyStmt, VObjectType::paConcurrent_assertion_item);
  if (!assertionItem) return false;

  NodeId firstChild = fC->Child(assertionItem);
  if (!firstChild || fC->Type(firstChild) != VObjectType::slStringConst)
    return false;

  NodeId moduleOrGenItem = FindAncestorOfType(
      fC, assertionItem, VObjectType::paModule_or_generate_item);
  if (!moduleOrGenItem) return false;

  NodeId firstItemChild = fC->Child(moduleOrGenItem);
  if (!firstItemChild) return true;

  return (fC->Type(firstItemChild) != VObjectType::paAttribute_instance);
}

static bool CheckProceduralCover(const FileContent* fC,
                                 NodeId coverPropertyStmt) {
  NodeId proceduralAssert = FindAncestorOfType(
      fC, coverPropertyStmt, VObjectType::paProcedural_assertion_statement);
  if (!proceduralAssert) return false;

  NodeId stmt =
      FindAncestorOfType(fC, proceduralAssert, VObjectType::paStatement);
  if (!stmt) return false;

  NodeId firstChild = fC->Child(stmt);
  if (!firstChild || fC->Type(firstChild) != VObjectType::slStringConst)
    return false;

  NodeId afterLabel = fC->Sibling(firstChild);
  if (!afterLabel) {
    return true;
  }

  VObjectType afterLabelType = fC->Type(afterLabel);
  return std::ranges::any_of(
      kValidAfterLabelTypes,
      [afterLabelType](VObjectType t) { return t == afterLabelType; });
}

static std::string_view ExtractLabelName(const FileContent* fC,
                                         NodeId coverPropertyStmt) {
  auto getFirstStringConst = [&](NodeId node) -> std::string_view {
    NodeId child = fC->Child(node);
    if (child && fC->Type(child) == VObjectType::slStringConst)
      return fC->SymName(child);
    return {};
  };

  NodeId assertionItem = FindAncestorOfType(
      fC, coverPropertyStmt, VObjectType::paConcurrent_assertion_item);
  if (assertionItem) {
    if (auto name = getFirstStringConst(assertionItem); !name.empty())
      return name;
  }

  return "<unknown>";
}

void CheckAssertionStatementAttributeInstance(const FileContent* fC,
                                              ErrorContainer* errors,
                                              SymbolTable* symbols) {
  if (!fC || !errors || !symbols) return;

  NodeId root = fC->getRootNode();
  if (!root) return;

  for (NodeId coverStmt :
       fC->sl_collect_all(root, VObjectType::paCover_property_statement)) {
    const bool kIsProcedural =
        FindAncestorOfType(fC, coverStmt,
                           VObjectType::paProcedural_assertion_statement) !=
        InvalidNodeId;

    const bool kHasViolation = kIsProcedural
                                   ? CheckProceduralCover(fC, coverStmt)
                                   : CheckModuleLevelCover(fC, coverStmt);

    if (!kHasViolation) continue;

    ReportError(fC, coverStmt, ExtractLabelName(fC, coverStmt),
                ErrorDefinition::LINT_ASSERTION_STATEMENT_ATTRIBUTE_INSTANCE,
                errors, symbols);
  }
}