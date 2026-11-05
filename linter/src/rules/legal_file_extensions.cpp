#include "rules/legal_file_extensions.h"

#include <Surelog/Common/FileSystem.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>

#include <cctype>
#include <filesystem>
#include <string>
#include <vector>

#include "main/lint_rules.h"
#include "utils/location_utils.h"

namespace SL = SURELOG;

namespace {
auto IsExtensionAllowed(const std::string& extension,
                        const std::vector<std::string>& allowedExtensions)
    -> bool {
  for (const std::string& allowed : allowedExtensions) {
    if (extension == allowed) {
      return true;
    }
  }
  return false;
}

}  // namespace

void CheckLegalFileExtensions(const SL::FileContent* fileContent,
                              SL::ErrorContainer* errors,
                              SL::SymbolTable* symbols) {
  if (fileContent == nullptr) {
    return;
  }
  const std::vector<std::string> allowedExtensions = {".sv", ".svh"};
  const std::filesystem::path filePath =
      SL::FileSystem::getInstance()->toPath(fileContent->getFileId());

  std::string extension = filePath.extension().string();
  for (char& symbol : extension) {
    symbol =
        static_cast<char>(std::tolower(static_cast<unsigned char>(symbol)));
  }

  if (!IsExtensionAllowed(extension, allowedExtensions)) {
    ReportError(fileContent, fileContent->getRootNode(), extension,
                verihogg_lint::LINT_ILLEGAL_FILE_EXTENSION, errors, symbols);
  }
}