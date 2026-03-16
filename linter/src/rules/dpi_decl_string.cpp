#include "rules/dpi_decl_string.h"

#include <Surelog/Design/Design.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <string>

#include "utils/location_utils.h"
#include "utils/string_utils.h"

namespace SL = SURELOG;

void CheckDpiDeclarationString(const SL::FileContent* fileContent,
                               SL::ErrorContainer* errors,
                               SL::SymbolTable* symbols) {
  if (fileContent == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }
  SL::NodeId root = fileContent->getRootNode();
  if (!root) {
    return;
  }

  for (SL::NodeId dpiId : fileContent->sl_collect_all(
           root, SL::VObjectType::paDpi_import_export)) {
    SL::NodeId importNode = fileContent->Child(dpiId);
    if (!importNode ||
        fileContent->Type(importNode) != SL::VObjectType::paIMPORT) {
      continue;
    }

    SL::NodeId stringNode = fileContent->Sibling(importNode);
    if (!stringNode ||
        fileContent->Type(stringNode) != SL::VObjectType::slStringLiteral) {
      continue;
    }

    std::string dpiStr = std::string(fileContent->SymName(stringNode));

    if (!dpiStr.empty() && dpiStr.front() == '"' && dpiStr.back() == '"') {
      dpiStr = dpiStr.substr(1, dpiStr.size() - 2);
    }

    dpiStr = Trim(dpiStr);

    if (dpiStr != "DPI-C" && dpiStr != "DPI") {
      ReportError(fileContent, stringNode, dpiStr,
                  SL::ErrorDefinition::LINT_DPI_DECLARATION_STRING, errors,
                  symbols);
    }
  }
}
