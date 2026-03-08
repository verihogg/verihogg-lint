#include "rules/assignment_pattern_context.h"

#include <algorithm>
#include <string_view>

#include "Surelog/Design/FileContent.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"
#include "Surelog/SourceCompile/VObjectTypes.h"
#include "utils/location_utils.h"
#include "utils/name_utils.h"

using namespace SURELOG;

static constexpr std::array kHardWrappers = {
    VObjectType::paAssignment_pattern,
    VObjectType::paAssignment_pattern_expression,
    VObjectType::paConstant_assignment_pattern_expression,
    VObjectType::paPrimary,
    VObjectType::paConstant_primary,
    VObjectType::paExpression,
    VObjectType::paConstant_expression,
    VObjectType::paConstant_mintypmax_expression,
    VObjectType::paConstant_param_expression,
};

static constexpr std::array kValidContexts = {
    VObjectType::paOperator_assignment,
    VObjectType::paBlocking_assignment,
    VObjectType::paNonblocking_assignment,
    VObjectType::paNet_assignment,
    VObjectType::paNet_decl_assignment,
    VObjectType::paVariable_decl_assignment,
    VObjectType::paParam_assignment,
    VObjectType::paContinuous_assign,
};

static constexpr std::array kAssignmentLvalueTypes = {
    VObjectType::paOperator_assignment,
    VObjectType::paBlocking_assignment,
    VObjectType::paNonblocking_assignment,
    VObjectType::paNet_assignment,
};

static constexpr std::array kLvalueChildTypes = {
    VObjectType::paVariable_lvalue,
    VObjectType::paNet_lvalue,
};

static constexpr std::array kExpressionContextTypes = {
    VObjectType::paCond_predicate,
    VObjectType::paLoop_statement,
    VObjectType::paCase_statement,
    VObjectType::paExpression,
};

static bool isHardWrapper(VObjectType type) {
  return std::ranges::find(kHardWrappers, type) != kHardWrappers.end();
}

static bool isValidAssignmentContext(VObjectType type) {
  return std::ranges::find(kValidContexts, type) != kValidContexts.end();
}

static bool isConditionalExpression(const FileContent* fC, NodeId exprNode) {
  for (NodeId ch = fC->Child(exprNode); ch; ch = fC->Sibling(ch)) {
    if (fC->Type(ch) == VObjectType::paQMARK) return true;
  }
  return false;
}

static NodeId findDirectContext(const FileContent* fC, NodeId patternNode) {
  NodeId current = fC->Parent(patternNode);
  while (current && isHardWrapper(fC->Type(current))) {
    if (fC->Type(current) == VObjectType::paExpression &&
        isConditionalExpression(fC, current))
      return current;
    current = fC->Parent(current);
  }

  if (current && fC->Type(current) == VObjectType::paExpression) {
    NodeId parent = fC->Parent(current);
    while (parent && fC->Type(parent) == VObjectType::paExpression)
      parent = fC->Parent(parent);
    if (parent && isValidAssignmentContext(fC->Type(parent))) return parent;
    return current;
  }

  return current;
}

static std::string_view findContextName(const FileContent* fC,
                                        NodeId patternNode) {
  auto nameFromFirstChild = [&](NodeId node,
                                VObjectType targetType) -> std::string_view {
    for (NodeId ch = fC->Child(node); ch; ch = fC->Sibling(ch)) {
      if (fC->Type(ch) != targetType) continue;
      std::string_view name = extractName(fC, ch, "");
      if (!name.empty()) return name;
      break;
    }
    return {};
  };

  for (NodeId current = fC->Parent(patternNode); current;
       current = fC->Parent(current)) {
    VObjectType type = fC->Type(current);

    if (std::ranges::find(kAssignmentLvalueTypes, type) !=
        kAssignmentLvalueTypes.end()) {
      for (NodeId ch = fC->Child(current); ch; ch = fC->Sibling(ch)) {
        if (std::ranges::find(kLvalueChildTypes, fC->Type(ch)) !=
            kLvalueChildTypes.end())
          return extractName(fC, ch, "unknown");
      }
    }

    if (type == VObjectType::paSubroutine_call) {
      NodeId nameNode = fC->Child(current);
      if (nameNode && fC->Type(nameNode) == VObjectType::slStringConst)
        return fC->SymName(nameNode);
    }

    if (std::ranges::find(kExpressionContextTypes, type) !=
        kExpressionContextTypes.end()) {
      NodeId searchRoot = (type == VObjectType::paCond_predicate)
                              ? fC->Child(current)
                              : current;
      if (!searchRoot) continue;

      if (auto name = nameFromFirstChild(searchRoot, VObjectType::paExpression);
          !name.empty())
        return name;
    }
  }
  return "<unknown>";
}

void checkAssignmentPatternContext(const FileContent* fC,
                                   ErrorContainer* errors,
                                   SymbolTable* symbols) {
  if (!fC || !errors || !symbols) return;

  NodeId root = fC->getRootNode();
  if (!root) return;

  auto patterns = fC->sl_collect_all(root, VObjectType::paAssignment_pattern);

  for (NodeId pat : patterns) {
    if (!pat) continue;

    NodeId ctx = findDirectContext(fC, pat);

    if (ctx && isValidAssignmentContext(fC->Type(ctx))) continue;

    std::string_view name = findContextName(fC, pat);
    reportError(fC, pat, name, ErrorDefinition::LINT_ASSIGNMENT_PATTERN_CONTEXT,
                errors, symbols);
  }
}