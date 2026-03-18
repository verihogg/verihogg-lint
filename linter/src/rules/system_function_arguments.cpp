#include "rules/system_function_arguments.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/ErrorReporting/ErrorDefinition.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <algorithm>
#include <array>
#include <string>
#include <string_view>
#include <utility>

#include "utils/location_utils.h"

namespace SL = SURELOG;

static constexpr std::array<std::pair<std::string_view, int>, 10> kMaxArgs = {{
    {"typename", 1},
    {"bits", 1},
    {"isunknown", 1},
    {"signed", 1},
    {"unsigned", 1},
    {"size", 2},
    {"high", 2},
    {"low", 2},
    {"left", 2},
    {"right", 2},
}};

namespace {
auto CountArguments(const SL::FileContent* fileContent, SL::NodeId listOfArgs)
    -> int {
  if (!listOfArgs) {
    return 0;
  }

  int count = 0;
  for (SL::NodeId chr = fileContent->Child(listOfArgs); chr;
       chr = fileContent->Sibling(chr)) {
    SL::VObjectType const type = fileContent->Type(chr);
    if (type == SL::VObjectType::paExpression ||
        type == SL::VObjectType::paArgument) {
      ++count;
    }
  }
  return count;
}

struct SysFuncCall {
  SL::NodeId callNode;
  SL::NodeId listArgs;
  std::string funcName;
};

auto ExtractFromDollarCall(const SL::FileContent* fileContent,
                           SL::NodeId callNode, SysFuncCall& out) -> bool {
  SL::NodeId const dollarKw = fileContent->Child(callNode);
  if (!dollarKw ||
      fileContent->Type(dollarKw) != SL::VObjectType::paDollar_keyword) {
    {
      return false;
    }
  }

  SL::NodeId const nameNode = fileContent->Sibling(dollarKw);
  if (!nameNode ||
      fileContent->Type(nameNode) != SL::VObjectType::slStringConst) {
    {
      return false;
    }
  }

  SL::NodeId const listArgs = fileContent->Sibling(nameNode);
  if (!listArgs ||
      fileContent->Type(listArgs) != SL::VObjectType::paList_of_arguments) {
    {
      return false;
    }
  }

  out.callNode = callNode;
  out.listArgs = listArgs;
  out.funcName = std::string(fileContent->SymName(nameNode));
  return true;
}

auto ExtractFromSystemTask(const SL::FileContent* fileContent,
                           SL::NodeId callNode, SysFuncCall& out) -> bool {
  SL::NodeId const taskNames = fileContent->Child(callNode);
  if (!taskNames ||
      fileContent->Type(taskNames) != SL::VObjectType::paSystem_task_names) {
    return false;
  }

  SL::NodeId const nameNode = fileContent->Child(taskNames);
  if (!nameNode ||
      fileContent->Type(nameNode) != SL::VObjectType::slStringConst) {
    return false;
  }

  SL::NodeId const listArgs = fileContent->Sibling(taskNames);
  if (!listArgs ||
      fileContent->Type(listArgs) != SL::VObjectType::paList_of_arguments) {
    return false;
  }

  std::string_view const rawName = fileContent->SymName(nameNode);
  std::string const funcName = (!rawName.empty() && rawName[0] == '$')
                                   ? std::string(rawName.substr(1))
                                   : std::string(rawName);

  out.callNode = callNode;
  out.listArgs = listArgs;
  out.funcName = funcName;
  return true;
}

void CheckCall(const SL::FileContent* fileContent, const SysFuncCall& call,
               SL::ErrorContainer* errors, SL::SymbolTable* symbols) {
  const auto* const kEntry = std::ranges::find_if(
      kMaxArgs, [&](const auto& pair) { return pair.first == call.funcName; });
  if (kEntry == kMaxArgs.end()) {
    return;
  }

  int const maxAllowed = kEntry->second;
  int const actual =
      static_cast<int>(CountArguments(fileContent, call.listArgs));

  if (actual <= maxAllowed) {
    return;
  }

  std::string const symbolName =
      "$" + call.funcName + " is " + std::to_string(maxAllowed);

  ReportError(fileContent, call.callNode, symbolName,
              SL::ErrorDefinition::LINT_SYSTEM_FUNCTION_ARGUMENTS, errors,
              symbols);
}
}  // namespace

void CheckSystemFunctionArguments(const SL::FileContent* fileContent,
                                  SL::ErrorContainer* errors,
                                  SL::SymbolTable* symbols) {
  if (fileContent == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }

  SL::NodeId const root = fileContent->getRootNode();
  if (!root) {
    return;
  }

  for (SL::NodeId const node : fileContent->sl_collect_all(
           root, SL::VObjectType::paComplex_func_call)) {
    SysFuncCall call;
    if (ExtractFromDollarCall(fileContent, node, call)) {
      CheckCall(fileContent, call, errors, symbols);
    }
  }

  for (SL::NodeId const node :
       fileContent->sl_collect_all(root, SL::VObjectType::paSystem_task)) {
    SysFuncCall call;
    if (ExtractFromSystemTask(fileContent, node, call)) {
      CheckCall(fileContent, call, errors, symbols);
    }
  }

  for (SL::NodeId const node :
       fileContent->sl_collect_all(root, SL::VObjectType::paSubroutine_call)) {
    SysFuncCall call;
    if (ExtractFromDollarCall(fileContent, node, call)) {
      CheckCall(fileContent, call, errors, symbols);
    }
  }
}