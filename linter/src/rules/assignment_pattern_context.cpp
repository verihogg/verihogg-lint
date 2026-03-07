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

static bool isHardWrapper(VObjectType type) {
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
  return std::find(kHardWrappers.begin(), kHardWrappers.end(), type) !=
         kHardWrappers.end();
}

static bool isValidAssignmentContext(VObjectType type) {
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
  return std::find(kValidContexts.begin(), kValidContexts.end(), type) !=
         kValidContexts.end();
}

static NodeId findDirectContext(const FileContent* fC, NodeId patternNode) {
  NodeId current = fC->Parent(patternNode);

  while (current && isHardWrapper(fC->Type(current))) {
    current = fC->Parent(current);
  }

  if (current && fC->Type(current) == VObjectType::paExpression) {
    NodeId parent = fC->Parent(current);
    if (parent && isValidAssignmentContext(fC->Type(parent))) {
      return parent;
    }
    return current;
  }

  return current;
}

static std::string_view findContextName(const FileContent* fC,
                                        NodeId patternNode) {
  NodeId current = fC->Parent(patternNode);
  while (current) {
    VObjectType type = fC->Type(current);

    if (type == VObjectType::paOperator_assignment ||
        type == VObjectType::paBlocking_assignment ||
        type == VObjectType::paNonblocking_assignment ||
        type == VObjectType::paNet_assignment) {
      for (NodeId ch = fC->Child(current); ch; ch = fC->Sibling(ch)) {
        VObjectType ct = fC->Type(ch);
        if (ct == VObjectType::paVariable_lvalue ||
            ct == VObjectType::paNet_lvalue) {
          return extractName(fC, ch, "<unknown>");
        }
      }
    }

    if (type == VObjectType::paExpression) {
      for (NodeId ch = fC->Child(current); ch; ch = fC->Sibling(ch)) {
        if (fC->Type(ch) != VObjectType::paExpression) continue;
        std::string_view name = extractName(fC, ch, "");
        if (!name.empty()) return name;
      }
    }

    if (type == VObjectType::paSubroutine_call) {
      NodeId nameNode = fC->Child(current);
      if (nameNode && fC->Type(nameNode) == VObjectType::slStringConst)
        return fC->SymName(nameNode);
    }

    current = fC->Parent(current);
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
