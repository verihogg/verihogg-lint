#include "rules/system_function_arguments.h"

#include <string>
#include <string_view>
#include <unordered_map>

#include "Surelog/Design/FileContent.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"
#include "Surelog/SourceCompile/VObjectTypes.h"
#include "utils/ast_utils.h"
#include "utils/location_utils.h"

using namespace SURELOG;

static const std::unordered_map<std::string_view, int> kMaxArgs = {
    {"typename", 1}, {"bits", 1},  {"isunknown", 1}, {"signed", 1},
    {"unsigned", 1}, {"size", 2},  {"high", 2},      {"low", 2},
    {"left", 2},     {"right", 2},
};

static int countArguments(const FileContent* fC, NodeId listOfArgs) {
  if (!listOfArgs) return 0;

  int count = 0;
  for (NodeId ch = fC->Child(listOfArgs); ch; ch = fC->Sibling(ch)) {
    VObjectType t = fC->Type(ch);
    if (t == VObjectType::paExpression || t == VObjectType::paArgument) {
      ++count;
    }
  }
  return count;
}

struct SysFuncCall {
  NodeId callNode;
  NodeId listArgs;
  std::string funcName;
};

static bool extractFromDollarCall(const FileContent* fC, NodeId callNode,
                                  SysFuncCall& out) {
  NodeId dollarKw = fC->Child(callNode);
  if (!dollarKw || fC->Type(dollarKw) != VObjectType::paDollar_keyword) {
    return false;
  }

  NodeId nameNode = fC->Sibling(dollarKw);
  if (!nameNode || fC->Type(nameNode) != VObjectType::slStringConst) {
    return false;
  }

  NodeId listArgs = fC->Sibling(nameNode);
  if (!listArgs || fC->Type(listArgs) != VObjectType::paList_of_arguments) {
    return false;
  }

  out.callNode = callNode;
  out.listArgs = listArgs;
  out.funcName = std::string(fC->SymName(nameNode));
  return true;
}

static bool extractFromSystemTask(const FileContent* fC, NodeId callNode,
                                  SysFuncCall& out) {
  NodeId taskNames = fC->Child(callNode);
  if (!taskNames || fC->Type(taskNames) != VObjectType::paSystem_task_names) {
    return false;
  }

  NodeId nameNode = fC->Child(taskNames);
  if (!nameNode || fC->Type(nameNode) != VObjectType::slStringConst) {
    return false;
  }

  NodeId listArgs = fC->Sibling(taskNames);
  if (!listArgs || fC->Type(listArgs) != VObjectType::paList_of_arguments) {
    return false;
  }

  std::string_view rawName = fC->SymName(nameNode);
  std::string funcName = (!rawName.empty() && rawName[0] == '$')
                             ? std::string(rawName.substr(1))
                             : std::string(rawName);

  out.callNode = callNode;
  out.listArgs = listArgs;
  out.funcName = funcName;
  return true;
}

static void checkCall(const FileContent* fC, const SysFuncCall& call,
                      ErrorContainer* errors, SymbolTable* symbols) {
  auto it = kMaxArgs.find(call.funcName);
  if (it == kMaxArgs.end()) return;

  int maxAllowed = it->second;
  int actual = countArguments(fC, call.listArgs);

  if (actual <= maxAllowed) return;

  // Формируем единый символ: "$typename is 1"
  std::string symbolName =
      "$" + call.funcName + " is " + std::to_string(maxAllowed);

  reportError(fC, call.callNode, symbolName,
              ErrorDefinition::LINT_SYSTEM_FUNCTION_ARGUMENTS, errors, symbols);
}

void checkSystemFunctionArguments(const FileContent* fC, ErrorContainer* errors,
                                  SymbolTable* symbols) {
  if (!fC || !errors || !symbols) return;

  NodeId root = fC->getRootNode();
  if (!root) return;

  for (NodeId node :
       fC->sl_collect_all(root, VObjectType::paComplex_func_call)) {
    SysFuncCall call;
    if (extractFromDollarCall(fC, node, call)) {
      checkCall(fC, call, errors, symbols);
    }
  }

  for (NodeId node : fC->sl_collect_all(root, VObjectType::paSystem_task)) {
    SysFuncCall call;
    if (extractFromSystemTask(fC, node, call)) {
      checkCall(fC, call, errors, symbols);
    }
  }

  for (NodeId node : fC->sl_collect_all(root, VObjectType::paSubroutine_call)) {
    SysFuncCall call;
    if (extractFromDollarCall(fC, node, call)) {
      checkCall(fC, call, errors, symbols);
    }
  }
}