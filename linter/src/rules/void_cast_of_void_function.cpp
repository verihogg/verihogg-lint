#include "rules/void_cast_of_void_function.h"

#include <Surelog/Common/FileSystem.h>
#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <cstdint>
#include <iostream>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

#include "fix/fix_it.h"
#include "main/lint_diagnostics.h"
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

struct ClosingParen {
  unsigned line = 0;
  unsigned col = 0;
};

auto FindMatchingCloseParen(const std::vector<std::string>& srcLines,
                            unsigned openParenLine, unsigned openParenCol)
    -> ClosingParen {
  if (openParenLine == 0 || openParenLine > srcLines.size()) {
    return {};
  }

  int depth = 0;
  for (unsigned lineIdx = openParenLine; lineIdx <= srcLines.size();
       ++lineIdx) {
    const std::string& line = srcLines.at(lineIdx - 1);
    const size_t startCol =
        (lineIdx == openParenLine) ? static_cast<size_t>(openParenCol - 1) : 0;

    for (size_t colIdx = startCol; colIdx < line.size(); ++colIdx) {
      const char ch = line.at(colIdx);
      if (ch == '(') {
        ++depth;
      } else if (ch == ')') {
        --depth;
        if (depth == 0) {
          return ClosingParen{.line = lineIdx,
                              .col = static_cast<unsigned>(colIdx + 1)};
        }
      }
    }
  }
  return {};
}

}  // namespace

auto CheckVoidCastOfVoidFunctionFixable(const SL::FileContent* fileContent,
                                        SL::ErrorContainer* errors,
                                        SL::SymbolTable* symbols,
                                        FixSourceManager& /*sm*/)
    -> std::vector<LintDiagnostic> {
  std::vector<LintDiagnostic> diags;

  if (fileContent == nullptr || errors == nullptr || symbols == nullptr) {
    return diags;
  }

  SL::NodeId const kRoot = fileContent->getRootNode();
  if (!kRoot) {
    return diags;
  }

  const std::string filepath = GetFixFilepath(fileContent);
  if (filepath.empty()) {
    return diags;
  }

  auto const kVoidFunctions = CollectVoidFunctionNames(fileContent);
  if (kVoidFunctions.empty()) {
    return diags;
  }

  std::vector<std::string> srcLines;
  SL::FileSystem* const fileSystem = SL::FileSystem::getInstance();
  if (fileSystem != nullptr) {
    (void)fileSystem->readLines(fileContent->getFileId(), srcLines);
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
    if (!kVoidFunctions.contains(kFuncName)) {
      continue;
    }

    ReportError(fileContent, kCall, kFuncName,
                verihogg_lint::LINT_VOID_CAST_OF_VOID_FUNCTION, errors,
                symbols);

    const unsigned stmtLine = fileContent->Line(kStmt);
    const unsigned stmtCol = GetColumnSafe(fileContent, kStmt);
    const unsigned callCol = GetColumnSafe(fileContent, kCall);
    if (stmtLine == 0 || stmtCol == 0 || callCol == 0) {
      continue;
    }

    const ClosingParen closing =
        FindMatchingCloseParen(srcLines, stmtLine, callCol - 1);
    if (closing.line == 0 || closing.col == 0 || closing.line != stmtLine) {
      continue;
    }

    const std::string& srcLine = srcLines.at(stmtLine - 1);
    if (callCol == 0 || callCol > srcLine.size() ||
        closing.col > srcLine.size() + 1) {
      continue;
    }
    const std::string inner =
        srcLine.substr(callCol - 1, closing.col - callCol);

    const FixRange replace_range{
        .begin =
            FixLocation{.filename = filepath, .line = stmtLine, .col = stmtCol},
        .end =
            FixLocation{
                .filename = filepath, .line = stmtLine, .col = closing.col + 1},
    };

    LintDiagnostic d;
    d.filepath = filepath;
    d.line = stmtLine;
    d.col = stmtCol;
    d.message = std::string(kFuncName) + ": remove void'(...)";

    try {
      d.fixes.push_back(
          FixIt::Replace(replace_range, inner, "remove void'(...)"));
    } catch (const std::exception& e) {
      std::cerr << "autofix: cannot create fix at " << filepath << ":"
                << stmtLine << ":" << stmtCol << ": " << e.what() << "\n";
      diags.push_back(std::move(d));
      continue;
    }

    diags.push_back(std::move(d));
  }

  return diags;
}
