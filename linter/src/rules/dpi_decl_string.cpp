#include "rules/dpi_decl_string.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/Design.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <string>

#include "main/lint_rules.h"
#include "utils/location_utils.h"
#include "utils/string_utils.h"

namespace SL = SURELOG;

void CheckDpiDeclarationString(const SL::FileContent* fileContent,
                               SL::ErrorContainer* errors,
                               SL::SymbolTable* symbols) {
  if (fileContent == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }
  SL::NodeId const kRoot = fileContent->getRootNode();
  if (!kRoot) {
    return;
  }

  for (SL::NodeId const kDpiId : fileContent->sl_collect_all(
           kRoot, SL::VObjectType::paDpi_import_export)) {
    SL::NodeId const kImportNode = fileContent->Child(kDpiId);
    if (!kImportNode ||
        fileContent->Type(kImportNode) != SL::VObjectType::paIMPORT) {
      continue;
    }

    SL::NodeId const kStringNode = fileContent->Sibling(kImportNode);
    if (!kStringNode ||
        fileContent->Type(kStringNode) != SL::VObjectType::slStringLiteral) {
      continue;
    }

    std::string dpiStr = std::string(fileContent->SymName(kStringNode));

    if (!dpiStr.empty() && dpiStr.front() == '"' && dpiStr.back() == '"') {
      dpiStr = dpiStr.substr(1, dpiStr.size() - 2);
    }

    dpiStr = Trim(dpiStr);

    if (dpiStr != "DPI-C" && dpiStr != "DPI") {
      ReportError(fileContent, kStringNode, dpiStr,
                  verihogg_lint::LINT_DPI_DECLARATION_STRING, errors, symbols);
    }
  }
}
