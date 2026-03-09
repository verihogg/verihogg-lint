#include "rules/dpi_decl_string.h"

#include <string>

#include "Surelog/Design/Design.h"
#include "Surelog/Design/FileContent.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"
#include "Surelog/SourceCompile/VObjectTypes.h"
#include "utils/location_utils.h"
#include "utils/string_utils.h"

using namespace SURELOG;

void CheckDpiDeclarationString(const FileContent* fC, ErrorContainer* errors,
                               SymbolTable* symbols) {
  if (!fC || !errors || !symbols) return;
  NodeId root = fC->getRootNode();
  if (!root) return;

  for (NodeId dpiId :
       fC->sl_collect_all(root, VObjectType::paDpi_import_export)) {
    NodeId importNode = fC->Child(dpiId);
    if (!importNode || fC->Type(importNode) != VObjectType::paIMPORT) continue;

    NodeId stringNode = fC->Sibling(importNode);
    if (!stringNode || fC->Type(stringNode) != VObjectType::slStringLiteral)
      continue;

    std::string dpiStr = std::string(fC->SymName(stringNode));

    if (!dpiStr.empty() && dpiStr.front() == '"' && dpiStr.back() == '"') {
      dpiStr = dpiStr.substr(1, dpiStr.size() - 2);
    }

    dpiStr = Trim(dpiStr);

    if (dpiStr != "DPI-C" && dpiStr != "DPI") {
      ReportError(fC, stringNode, dpiStr,
                  ErrorDefinition::LINT_DPI_DECLARATION_STRING, errors,
                  symbols);
    }
  }
}
