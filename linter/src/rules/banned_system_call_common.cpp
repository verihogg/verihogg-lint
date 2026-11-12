#include "rules/banned_system_call_common.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <algorithm>
#include <array>
#include <span>
#include <string>
#include <string_view>

#include "main/lint_rules.h"
#include "utils/location_utils.h"

namespace SL = SURELOG;

namespace {

auto GetDollarCallName(const SL::FileContent* fileContent, SL::NodeId callNode)
    -> std::string_view {
  SL::NodeId const kDollarKw = fileContent->Child(callNode);
  if (!kDollarKw ||
      fileContent->Type(kDollarKw) != SL::VObjectType::paDollar_keyword) {
    return {};
  }
  SL::NodeId const kNameNode = fileContent->Sibling(kDollarKw);
  if (!kNameNode ||
      fileContent->Type(kNameNode) != SL::VObjectType::slStringConst) {
    return {};
  }
  return fileContent->SymName(kNameNode);
}

auto GetSystemTaskName(const SL::FileContent* fileContent, SL::NodeId taskNode)
    -> std::string_view {
  SL::NodeId const kTaskNames = fileContent->Child(taskNode);
  if (!kTaskNames ||
      fileContent->Type(kTaskNames) != SL::VObjectType::paSystem_task_names) {
    return {};
  }
  SL::NodeId const kNameNode = fileContent->Child(kTaskNames);
  if (!kNameNode ||
      fileContent->Type(kNameNode) != SL::VObjectType::slStringConst) {
    return {};
  }
  std::string_view name = fileContent->SymName(kNameNode);
  if (name.starts_with('$')) {
    name.remove_prefix(1);
  }
  return name;
}

}  // namespace

void CheckBannedSystemCallsImpl(const SL::FileContent* fileContent,
                                SL::ErrorContainer* errors,
                                SL::SymbolTable* symbols,
                                std::span<const std::string_view> bannedNames,
                                verihogg_lint::LintId ruleId) {
  if (fileContent == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }

  SL::NodeId const kRoot = fileContent->getRootNode();
  if (!kRoot) {
    return;
  }

  auto reportIfBanned = [&](SL::NodeId callNode, std::string_view name) {
    if (name.empty() ||
        std::ranges::find(bannedNames, name) == bannedNames.end()) {
      return;
    }
    ReportError(fileContent, callNode, "$" + std::string(name), ruleId, errors,
                symbols);
  };

  static constexpr std::array kDollarCallTypes = {
      SL::VObjectType::paSubroutine_call,
      SL::VObjectType::paComplex_func_call,
  };
  for (SL::VObjectType const kType : kDollarCallTypes) {
    for (SL::NodeId const kNode : fileContent->sl_collect_all(kRoot, kType)) {
      reportIfBanned(kNode, GetDollarCallName(fileContent, kNode));
    }
  }

  for (SL::NodeId const kNode :
       fileContent->sl_collect_all(kRoot, SL::VObjectType::paSystem_task)) {
    reportIfBanned(kNode, GetSystemTaskName(fileContent, kNode));
  }
}
