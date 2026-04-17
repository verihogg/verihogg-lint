#include "rules/void_cast_of_void_function.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <cstdint>
#include <string_view>
#include <unordered_set>

#include "main/lint_rules.h"
#include "utils/ast_utils.h"
#include "utils/location_utils.h"

namespace SL = SURELOG;

namespace {

constexpr uint16_t kVoidCastPrefixLen = 6;

auto IsVoidReturnType(const SL::FileContent* fc,
                      SL::NodeId funcDataTypeOrImplicit) -> bool {
  SL::NodeId const kFuncDataType = FindChildOfType(
      fc, funcDataTypeOrImplicit, SL::VObjectType::paFunction_data_type);
  if (!kFuncDataType) {
    return false;
  }
  return !FindChildOfType(fc, kFuncDataType, SL::VObjectType::paData_type);
}

auto CollectVoidFunctionNames(const SL::FileContent* fc)
    -> std::unordered_set<std::string_view> {
  std::unordered_set<std::string_view> result;

  SL::NodeId const kRoot = fc->getRootNode();
  if (!kRoot) {
    return result;
  }

  for (SL::NodeId const kFuncDecl :
       fc->sl_collect_all(kRoot, SL::VObjectType::paFunction_declaration)) {
    SL::NodeId const kBodyDecl = FindChildOfType(
        fc, kFuncDecl, SL::VObjectType::paFunction_body_declaration);
    if (!kBodyDecl) {
      continue;
    }

    SL::NodeId const kFdtoi = FindChildOfType(
        fc, kBodyDecl, SL::VObjectType::paFunction_data_type_or_implicit);
    if (!kFdtoi) {
      continue;
    }

    if (!IsVoidReturnType(fc, kFdtoi)) {
      continue;
    }

    SL::NodeId const kFuncName =
        FindSiblingOfType(fc, kFdtoi, SL::VObjectType::slStringConst);
    if (!kFuncName) {
      continue;
    }

    result.insert(fc->SymName(kFuncName));
  }

  return result;
}

auto IsVoidCastStatement(const SL::FileContent* fc, SL::NodeId stmtNode,
                         SL::NodeId callNode) -> bool {
  if (fc->Line(stmtNode) != fc->Line(callNode)) {
    return false;
  }
  return fc->Column(callNode) == fc->Column(stmtNode) + kVoidCastPrefixLen;
}

}  // namespace

void CheckVoidCastOfVoidFunction(const SL::FileContent* fileContent,
                                 SL::ErrorContainer* errors,
                                 SL::SymbolTable* symbols) {
  if (fileContent == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }

  SL::NodeId const kRoot = fileContent->getRootNode();
  if (!kRoot) {
    return;
  }

  auto const kVoidFunctions = CollectVoidFunctionNames(fileContent);
  if (kVoidFunctions.empty()) {
    return;
  }

  for (SL::NodeId const kStmt : fileContent->sl_collect_all(
           kRoot, SL::VObjectType::paSubroutine_call_statement)) {
    SL::NodeId const kCall = fileContent->Child(kStmt);
    if (!kCall ||
        fileContent->Type(kCall) != SL::VObjectType::paSubroutine_call) {
      continue;
    }

    if (!IsVoidCastStatement(fileContent, kStmt, kCall)) {
      continue;
    }

    SL::NodeId const kFuncNameNode = fileContent->Child(kCall);
    if (!kFuncNameNode ||
        fileContent->Type(kFuncNameNode) != SL::VObjectType::slStringConst) {
      continue;
    }

    std::string_view const kFuncName = fileContent->SymName(kFuncNameNode);
    if (kVoidFunctions.contains(kFuncName)) {
      ReportError(fileContent, kCall, kFuncName,
                  verihogg_lint::LINT_VOID_CAST_OF_VOID_FUNCTION, errors,
                  symbols);
    }
  }
}
