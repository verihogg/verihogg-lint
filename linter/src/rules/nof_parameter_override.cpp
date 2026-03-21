#include "rules/nof_parameter_override.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/Design.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <string_view>
#include <unordered_map>

#include "main/lint_rules.h"
#include "utils/ast_utils.h"
#include "utils/design_utils.h"
#include "utils/location_utils.h"

namespace SL = SURELOG;

namespace {
auto CountInSiblingChain(const SL::FileContent* fileContent,
                         SL::NodeId firstNode, SL::VObjectType targetType)
    -> int {
  int count = 0;
  for (SL::NodeId cur = firstNode; cur; cur = fileContent->Sibling(cur)) {
    if (fileContent->Type(cur) == targetType) {
      ++count;
    }
  }
  return count;
}

auto CountModuleParams(const SL::FileContent* fileContent,
                       SL::NodeId moduleDecl) -> int {
  SL::NodeId const header = fileContent->Child(moduleDecl);
  if (!header) {
    return 0;
  }

  SL::NodeId const portList = FindChildOfType(
      fileContent, header, SL::VObjectType::paParameter_port_list);

  if (!portList) {
    return 0;
  }

  SL::NodeId const firstDecl = fileContent->Child(portList);
  if (!firstDecl) {
    return 0;
  }

  return CountInSiblingChain(fileContent, firstDecl,
                             SL::VObjectType::paParameter_port_declaration);
}

auto BuildModuleParamMap(const SL::FileContent* fileContent)
    -> std::unordered_map<std::string_view, int> {
  std::unordered_map<std::string_view, int> result;

  SL::NodeId const root = fileContent->getRootNode();
  if (!root) {
    return result;
  }

  for (SL::NodeId const moduleDecl : fileContent->sl_collect_all(
           root, SL::VObjectType::paModule_declaration)) {
    SL::NodeId const header = fileContent->Child(moduleDecl);
    if (!header) {
      continue;
    }
    SL::NodeId const keyword = fileContent->Child(header);
    if (!keyword) {
      continue;
    }
    SL::NodeId const nameNode = fileContent->Sibling(keyword);
    if (!nameNode ||
        fileContent->Type(nameNode) != SL::VObjectType::slStringConst) {
      continue;
    }

    std::string_view const moduleName = fileContent->SymName(nameNode);
    result[moduleName] = CountModuleParams(fileContent, moduleDecl);
  }

  return result;
}

auto CountOrderedOverrides(const SL::FileContent* fileContent,
                           SL::NodeId instNode) -> int {
  SL::NodeId const moduleNameNode = fileContent->Child(instNode);
  if (!moduleNameNode) {
    return -1;
  }

  SL::NodeId const paramValueAssign =
      FindSiblingOfType(fileContent, moduleNameNode,
                        SL::VObjectType::paParameter_value_assignment);

  if (!paramValueAssign) {
    return -1;
  }

  SL::NodeId const list = fileContent->Child(paramValueAssign);
  if (!list || fileContent->Type(list) !=
                   SL::VObjectType::paList_of_parameter_assignments) {
    return -1;
  }

  SL::NodeId const firstAssign = fileContent->Child(list);
  if (!firstAssign) {
    return -1;
  }

  if (fileContent->Type(firstAssign) ==
      SL::VObjectType::paNamed_parameter_assignment) {
    return -1;
  }

  return CountInSiblingChain(fileContent, firstAssign,
                             SL::VObjectType::paOrdered_parameter_assignment);
}

void CheckInstantiationsInFile(
    const SL::FileContent* fileContent,
    const std::unordered_map<std::string_view, int>& globalParamMap,
    SL::ErrorContainer* errors, SL::SymbolTable* symbols) {
  SL::NodeId const root = fileContent->getRootNode();
  if (!root) {
    return;
  }

  for (SL::NodeId const inst : fileContent->sl_collect_all(
           root, SL::VObjectType::paModule_instantiation)) {
    int const overrideCount = CountOrderedOverrides(fileContent, inst);
    if (overrideCount < 0) {
      continue;
    }

    SL::NodeId const moduleNameNode = fileContent->Child(inst);
    if (!moduleNameNode) {
      continue;
    }
    std::string_view const moduleName = fileContent->SymName(moduleNameNode);

    auto const it = globalParamMap.find(moduleName);
    if (it == globalParamMap.end()) {
      continue;
    }

    if (overrideCount == it->second) {
      continue;
    }

    SL::NodeId badNode = fileContent->Sibling(moduleNameNode);
    if (!badNode) {
      badNode = inst;
    }

    ReportError(fileContent, badNode, moduleName,
                verihogg_lint::LINT_NOF_PARAMETER_OVERRIDE, errors, symbols);
  }
}
}  // namespace

void CheckNofParameterOverrides(SL::Design* design, SL::ErrorContainer* errors,
                                SL::SymbolTable* symbols) {
  if (design == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }

  std::unordered_map<std::string_view, int> globalParamMap;
  DesignUtils::ForEachFileContent(design, [&](const SL::FileContent* fc) {
    globalParamMap.merge(BuildModuleParamMap(fc));
  });

  for (auto& [name, fileContent] : design->getAllFileContents()) {
    if (fileContent == nullptr) {
      continue;
    }
    CheckInstantiationsInFile(fileContent, globalParamMap, errors, symbols);
  }
}