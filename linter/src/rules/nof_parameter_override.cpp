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
  SL::NodeId const kHeader = fileContent->Child(moduleDecl);
  if (!kHeader) {
    return 0;
  }

  SL::NodeId const kPortList = FindChildOfType(
      fileContent, kHeader, SL::VObjectType::paParameter_port_list);

  if (!kPortList) {
    return 0;
  }

  SL::NodeId const kFirstDecl = fileContent->Child(kPortList);
  if (!kFirstDecl) {
    return 0;
  }

  return CountInSiblingChain(fileContent, kFirstDecl,
                             SL::VObjectType::paParameter_port_declaration);
}

auto BuildModuleParamMap(const SL::FileContent* fileContent)
    -> std::unordered_map<std::string_view, int> {
  std::unordered_map<std::string_view, int> result;

  SL::NodeId const kRoot = fileContent->getRootNode();
  if (!kRoot) {
    return result;
  }

  for (SL::NodeId const kModuleDecl : fileContent->sl_collect_all(
           kRoot, SL::VObjectType::paModule_declaration)) {
    SL::NodeId const kHeader = fileContent->Child(kModuleDecl);
    if (!kHeader) {
      continue;
    }
    SL::NodeId const kEyword = fileContent->Child(kHeader);
    if (!kEyword) {
      continue;
    }
    SL::NodeId const kNameNode = fileContent->Sibling(kEyword);
    if (!kNameNode ||
        fileContent->Type(kNameNode) != SL::VObjectType::slStringConst) {
      continue;
    }

    std::string_view const kModuleName = fileContent->SymName(kNameNode);
    result[kModuleName] = CountModuleParams(fileContent, kModuleDecl);
  }

  return result;
}

auto CountOrderedOverrides(const SL::FileContent* fileContent,
                           SL::NodeId instNode) -> int {
  SL::NodeId const kModuleNameNode = fileContent->Child(instNode);
  if (!kModuleNameNode) {
    return -1;
  }

  SL::NodeId const kParamValueAssign =
      FindSiblingOfType(fileContent, kModuleNameNode,
                        SL::VObjectType::paParameter_value_assignment);

  if (!kParamValueAssign) {
    return -1;
  }

  SL::NodeId const kList = fileContent->Child(kParamValueAssign);
  if (!kList || fileContent->Type(kList) !=
                    SL::VObjectType::paList_of_parameter_assignments) {
    return -1;
  }

  SL::NodeId const kFirstAssign = fileContent->Child(kList);
  if (!kFirstAssign) {
    return -1;
  }

  if (fileContent->Type(kFirstAssign) ==
      SL::VObjectType::paNamed_parameter_assignment) {
    return -1;
  }

  return CountInSiblingChain(fileContent, kFirstAssign,
                             SL::VObjectType::paOrdered_parameter_assignment);
}

void CheckInstantiationsInFile(
    const SL::FileContent* fileContent,
    const std::unordered_map<std::string_view, int>& globalParamMap,
    SL::ErrorContainer* errors, SL::SymbolTable* symbols) {
  SL::NodeId const kRoot = fileContent->getRootNode();
  if (!kRoot) {
    return;
  }

  for (SL::NodeId const kInst : fileContent->sl_collect_all(
           kRoot, SL::VObjectType::paModule_instantiation)) {
    int const kOverrideCount = CountOrderedOverrides(fileContent, kInst);
    if (kOverrideCount < 0) {
      continue;
    }

    SL::NodeId const kModuleNameNode = fileContent->Child(kInst);
    if (!kModuleNameNode) {
      continue;
    }
    std::string_view const kModuleName = fileContent->SymName(kModuleNameNode);

    auto const kIt = globalParamMap.find(kModuleName);
    if (kIt == globalParamMap.end()) {
      continue;
    }

    if (kOverrideCount == kIt->second) {
      continue;
    }

    SL::NodeId badNode = fileContent->Sibling(kModuleNameNode);
    if (!badNode) {
      badNode = kInst;
    }

    ReportError(fileContent, badNode, kModuleName,
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
  DesignUtils::ForEachFileContent(design, [&](const SL::FileContent* fileCont) {
    globalParamMap.merge(BuildModuleParamMap(fileCont));
  });

  for (auto& [name, fileContent] : design->getAllFileContents()) {
    if (fileContent == nullptr) {
      continue;
    }
    CheckInstantiationsInFile(fileContent, globalParamMap, errors, symbols);
  }
}