#include "rules/include_without_path.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/Design.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <string>
#include <vector>

#include "main/lint_rules.h"
#include "utils/location_utils.h"

namespace SL = SURELOG;

namespace {

auto GetIncludePath(const SL::FileContent* ppFC, SL::NodeId directiveId)
    -> std::string {
  const SL::NodeId pathId =
      ppFC->sl_get(directiveId, SL::VObjectType::ppString);
  if (!pathId) {
    return "";
  }

  std::string path{ppFC->SymName(pathId)};

  if (path.size() >= 2 && path.front() == '"' && path.back() == '"') {
    path = path.substr(1, path.size() - 2);
  }

  return path;
}

auto ContainsPathSeparator(const std::string& path) -> bool {
  return path.find('/') != std::string::npos ||
         path.find('\\') != std::string::npos;
}

}  // namespace

void CheckIncludeWithoutPath(SL::Design* design, SL::ErrorContainer* errors,
                             SL::SymbolTable* symbols) {
  if (design == nullptr) {
    return;
  }

  const auto& ppContents = design->getAllPPFileContents();
  for (const auto& entry : ppContents) {
    const SL::FileContent* ppFC = entry.second;
    if (ppFC == nullptr) {
      continue;
    }

    const std::vector<SL::NodeId> directives = ppFC->sl_collect_all(
        ppFC->getRootNode(), SL::VObjectType::ppInclude_directive);

    for (const SL::NodeId directiveId : directives) {
      const std::string includePath = GetIncludePath(ppFC, directiveId);

      if (ContainsPathSeparator(includePath)) {
        ReportError(ppFC, directiveId, includePath,
                    verihogg_lint::LINT_INCLUDE_WITH_PATH, errors, symbols);
      }
    }
  }
}