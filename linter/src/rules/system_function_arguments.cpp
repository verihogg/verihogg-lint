#include "rules/system_function_arguments.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <algorithm>
#include <array>
#include <string>
#include <string_view>
#include <utility>

#include "main/lint_rules.h"
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
    SL::VObjectType const kType = fileContent->Type(chr);
    if (kType == SL::VObjectType::paExpression ||
        kType == SL::VObjectType::paArgument) {
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
  SL::NodeId const kDollarKw = fileContent->Child(callNode);
  if (!kDollarKw ||
      fileContent->Type(kDollarKw) != SL::VObjectType::paDollar_keyword) {
    {
      return false;
    }
  }

  SL::NodeId const kNameNode = fileContent->Sibling(kDollarKw);
  if (!kNameNode ||
      fileContent->Type(kNameNode) != SL::VObjectType::slStringConst) {
    {
      return false;
    }
  }

  SL::NodeId const kListArgs = fileContent->Sibling(kNameNode);
  if (!kListArgs ||
      fileContent->Type(kListArgs) != SL::VObjectType::paList_of_arguments) {
    {
      return false;
    }
  }

  out.callNode = callNode;
  out.listArgs = kListArgs;
  out.funcName = std::string(fileContent->SymName(kNameNode));
  return true;
}

auto ExtractFromSystemTask(const SL::FileContent* fileContent,
                           SL::NodeId callNode, SysFuncCall& out) -> bool {
  SL::NodeId const kTaskNames = fileContent->Child(callNode);
  if (!kTaskNames ||
      fileContent->Type(kTaskNames) != SL::VObjectType::paSystem_task_names) {
    return false;
  }

  SL::NodeId const kNameNode = fileContent->Child(kTaskNames);
  if (!kNameNode ||
      fileContent->Type(kNameNode) != SL::VObjectType::slStringConst) {
    return false;
  }

  SL::NodeId const kListArgs = fileContent->Sibling(kTaskNames);
  if (!kListArgs ||
      fileContent->Type(kListArgs) != SL::VObjectType::paList_of_arguments) {
    return false;
  }

  std::string_view const kRawName = fileContent->SymName(kNameNode);
  std::string const kFuncName = (!kRawName.empty() && kRawName[0] == '$')
                                    ? std::string(kRawName.substr(1))
                                    : std::string(kRawName);

  out.callNode = callNode;
  out.listArgs = kListArgs;
  out.funcName = kFuncName;
  return true;
}

void CheckCall(const SL::FileContent* fileContent, const SysFuncCall& call,
               SL::ErrorContainer* errors, SL::SymbolTable* symbols) {
  const auto* const kEntry = std::ranges::find_if(
      kMaxArgs, [&](const auto& pair) { return pair.first == call.funcName; });
  if (kEntry == kMaxArgs.end()) {
    return;
  }

  int const kMaxAllowed = kEntry->second;
  int const kActual = CountArguments(fileContent, call.listArgs);

  if (kActual <= kMaxAllowed) {
    return;
  }

  std::string const kSymbolName =
      "$" + call.funcName + " is " + std::to_string(kMaxAllowed);

  ReportError(fileContent, call.callNode, kSymbolName,
              verihogg_lint::LINT_SYSTEM_FUNCTION_ARGUMENTS, errors, symbols);
}
}  // namespace

void CheckSystemFunctionArguments(const SL::FileContent* fileContent,
                                  SL::ErrorContainer* errors,
                                  SL::SymbolTable* symbols) {
  if (fileContent == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }

  SL::NodeId const kRoot = fileContent->getRootNode();
  if (!kRoot) {
    return;
  }

  for (SL::NodeId const kNode : fileContent->sl_collect_all(
           kRoot, SL::VObjectType::paComplex_func_call)) {
    SysFuncCall call;
    if (ExtractFromDollarCall(fileContent, kNode, call)) {
      CheckCall(fileContent, call, errors, symbols);
    }
  }

  for (SL::NodeId const kNode :
       fileContent->sl_collect_all(kRoot, SL::VObjectType::paSystem_task)) {
    SysFuncCall call;
    if (ExtractFromSystemTask(fileContent, kNode, call)) {
      CheckCall(fileContent, call, errors, symbols);
    }
  }

  for (SL::NodeId const kNode :
       fileContent->sl_collect_all(kRoot, SL::VObjectType::paSubroutine_call)) {
    SysFuncCall call;
    if (ExtractFromDollarCall(fileContent, kNode, call)) {
      CheckCall(fileContent, call, errors, symbols);
    }
  }
}