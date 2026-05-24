#include "rules/undeclared_checker.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/Design.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

#include "main/lint_rules.h"
#include "utils/location_utils.h"
#include "utils/name_utils.h"

void CheckUndeclaredChecker(SURELOG::Design* design,
                            SURELOG::ErrorContainer* errors,
                            SURELOG::SymbolTable* symbols) {
  if (!design || !errors || !symbols) return;

  std::unordered_set<std::string> declaredCheckers;
  std::unordered_set<std::string> declaredModules;
  std::vector<std::pair<const SURELOG::FileContent*, SURELOG::NodeId>>
      instantiations;

  for (const auto& [_, fileContent] : design->getAllFileContents()) {
    if (!fileContent) continue;
    SURELOG::NodeId root = fileContent->getRootNode();
    if (!root) continue;

    for (SURELOG::NodeId modNode : fileContent->sl_collect_all(
             root, SURELOG::VObjectType::paModule_declaration)) {
      std::string_view name = ExtractName(fileContent, modNode);
      if (!name.empty()) declaredModules.emplace(name);
    }
    for (SURELOG::NodeId chkNode : fileContent->sl_collect_all(
             root, SURELOG::VObjectType::paChecker_declaration)) {
      std::string_view name = ExtractName(fileContent, chkNode);
      if (!name.empty()) declaredCheckers.emplace(name);
    }
  }

  for (const auto& [_, fileContent] : design->getAllFileContents()) {
    if (!fileContent) continue;
    SURELOG::NodeId root = fileContent->getRootNode();
    if (!root) continue;

    for (SURELOG::NodeId instNode : fileContent->sl_collect_all(
             root, SURELOG::VObjectType::paModule_instantiation)) {
      std::string instName;
      auto strings = fileContent->sl_collect_all(
          instNode, SURELOG::VObjectType::slStringConst);
      if (!strings.empty()) {
        instName = std::string(fileContent->SymName(strings.front()));
      }

      if (!instName.empty() &&
          declaredModules.find(instName) == declaredModules.end() &&
          declaredCheckers.find(instName) == declaredCheckers.end()) {
        instantiations.emplace_back(fileContent, instNode);
      }
    }
  }

  for (const auto& [fc, node] : instantiations) {
    std::string instName;
    auto strings =
        fc->sl_collect_all(node, SURELOG::VObjectType::slStringConst);
    if (!strings.empty()) {
      instName = std::string(fc->SymName(strings.front()));
    }
    ReportError(fc, node, instName, verihogg_lint::LINT_UNDECLARED_CHECKER,
                errors, symbols);
  }
}
