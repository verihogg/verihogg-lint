#include "rules/assignment_pattern_context.h"

#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <algorithm>
#include <string_view>

#include "utils/location_utils.h"
#include "utils/name_utils.h"

namespace SL = SURELOG;

static constexpr std::array kHardWrappers = {
    SL::VObjectType::paAssignment_pattern,
    SL::VObjectType::paAssignment_pattern_expression,
    SL::VObjectType::paConstant_assignment_pattern_expression,
    SL::VObjectType::paPrimary,
    SL::VObjectType::paConstant_primary,
    SL::VObjectType::paExpression,
    SL::VObjectType::paConstant_expression,
    SL::VObjectType::paConstant_mintypmax_expression,
    SL::VObjectType::paConstant_param_expression,
};

static constexpr std::array kValidContexts = {
    SL::VObjectType::paOperator_assignment,
    SL::VObjectType::paBlocking_assignment,
    SL::VObjectType::paNonblocking_assignment,
    SL::VObjectType::paNet_assignment,
    SL::VObjectType::paNet_decl_assignment,
    SL::VObjectType::paVariable_decl_assignment,
    SL::VObjectType::paParam_assignment,
    SL::VObjectType::paContinuous_assign,
};

static constexpr std::array kAssignmentLvalueTypes = {
    SL::VObjectType::paOperator_assignment,
    SL::VObjectType::paBlocking_assignment,
    SL::VObjectType::paNonblocking_assignment,
    SL::VObjectType::paNet_assignment,
};

static constexpr std::array kLvalueChildTypes = {
    SL::VObjectType::paVariable_lvalue,
    SL::VObjectType::paNet_lvalue,
};

static constexpr std::array kExpressionContextTypes = {
    SL::VObjectType::paCond_predicate,
    SL::VObjectType::paLoop_statement,
    SL::VObjectType::paCase_statement,
    SL::VObjectType::paExpression,
};

static auto IsHardWrapper(SL::VObjectType type) {
  return std::ranges::find(kHardWrappers, type) != kHardWrappers.end();
}

static auto IsValidAssignmentContext(SL::VObjectType type) {
  return std::ranges::find(kValidContexts, type) != kValidContexts.end();
}

static auto FindDirectContext(const SL::FileContent* fileContent,
                              SL::NodeId patternNode) -> SL::NodeId {
  SL::NodeId current = fileContent->Parent(patternNode);
  while (current && IsHardWrapper(fileContent->Type(current))) {
    current = fileContent->Parent(current);
  }
  return current;
}

static auto NameFromFirstChild(const SL::FileContent* fileContent,
                               SL::NodeId node, SL::VObjectType targetType)
    -> std::string_view {
  for (SL::NodeId ch = fileContent->Child(node); ch != SL::NodeId(0);
       ch = fileContent->Sibling(ch)) {
    if (fileContent->Type(ch) != targetType) {
      continue;
    }
    std::string_view name = ExtractName(fileContent, ch, "");
    if (!name.empty()) {
      return name;
    }
    break;
  }
  return {};
}

static auto NameFromLvalue(const SL::FileContent* fileContent,
                           SL::NodeId current) -> std::string_view {
  for (SL::NodeId ch = fileContent->Child(current); ch != SL::NodeId(0);
       ch = fileContent->Sibling(ch)) {
    if (std::ranges::find(kLvalueChildTypes, fileContent->Type(ch)) !=
        kLvalueChildTypes.end()) {
      return ExtractName(fileContent, ch, "unknown");
    }
  }
  return {};
}

static auto NameFromSubroutineCall(const SL::FileContent* fileContent,
                                   SL::NodeId current) -> std::string_view {
  SL::NodeId nameNode = fileContent->Child(current);
  if (nameNode != SL::NodeId(0) &&
      fileContent->Type(nameNode) == SL::VObjectType::slStringConst) {
    return fileContent->SymName(nameNode);
  }
  return {};
}

static auto NameFromExpressionContext(const SL::FileContent* fileContent,
                                      SL::NodeId current, SL::VObjectType type)
    -> std::string_view {
  SL::NodeId searchRoot = (type == SL::VObjectType::paCond_predicate)
                              ? fileContent->Child(current)
                              : current;
  if (searchRoot == SL::NodeId(0)) {
    return {};
  }
  return NameFromFirstChild(fileContent, searchRoot,
                            SL::VObjectType::paExpression);
}

static auto FindContextName(const SL::FileContent* fileContent,
                            SL::NodeId patternNode) -> std::string_view {
  for (SL::NodeId current = fileContent->Parent(patternNode);
       current != SL::NodeId(0); current = fileContent->Parent(current)) {
    SL::VObjectType type = fileContent->Type(current);

    if (std::ranges::find(kAssignmentLvalueTypes, type) !=
        kAssignmentLvalueTypes.end()) {
      std::string_view name = NameFromLvalue(fileContent, current);
      if (!name.empty()) {
        return name;
      }
    }

    if (type == SL::VObjectType::paSubroutine_call) {
      std::string_view name = NameFromSubroutineCall(fileContent, current);
      if (!name.empty()) {
        return name;
      }
    }

    if (std::ranges::find(kExpressionContextTypes, type) !=
        kExpressionContextTypes.end()) {
      std::string_view name =
          NameFromExpressionContext(fileContent, current, type);
      if (!name.empty()) {
        return name;
      }
    }
  }
  return "<unknown>";
}

void CheckAssignmentPatternContext(const SL::FileContent* fileContent,
                                   SL::ErrorContainer* errors,
                                   SL::SymbolTable* symbols) {
  if (fileContent == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }

  SL::NodeId root = fileContent->getRootNode();
  if (!root) {
    return;
  }

  auto patterns =
      fileContent->sl_collect_all(root, SL::VObjectType::paAssignment_pattern);

  for (SL::NodeId pat : patterns) {
    if (!pat) {
      continue;
    }

    SL::NodeId ctx = FindDirectContext(fileContent, pat);

    if (ctx && IsValidAssignmentContext(fileContent->Type(ctx))) {
      continue;
    }

    std::string_view name = FindContextName(fileContent, pat);
    ReportError(fileContent, pat, name,
                SL::ErrorDefinition::LINT_ASSIGNMENT_PATTERN_CONTEXT, errors,
                symbols);
  }
}