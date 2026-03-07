#include "rules/concatenation_multiplier.h"

#include <map>
#include <string_view>
#include <unordered_set>

#include "Surelog/Design/Design.h"
#include "Surelog/Design/FileContent.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"
#include "Surelog/SourceCompile/VObjectTypes.h"
#include "utils/location_utils.h"
#include "utils/name_utils.h"

using namespace SURELOG;

std::unordered_set<std::string_view> collectConstantParameters(
    const FileContent* fC) {
  std::unordered_set<std::string_view> constants;

  NodeId root = fC->getRootNode();
  collectNames(fC, root, VObjectType::paParameter_declaration,
               VObjectType::paParam_assignment, constants);
  collectNames(fC, root, VObjectType::paLocal_parameter_declaration,
               VObjectType::paParam_assignment, constants);
  return constants;
}

std::unordered_set<std::string_view> collectVariables(const FileContent* fC) {
  std::unordered_set<std::string_view> variables;

  NodeId root = fC->getRootNode();
  collectNames(fC, root, VObjectType::paVariable_declaration,
               VObjectType::paVariable_decl_assignment, variables);
  collectNames(fC, root, VObjectType::paData_declaration,
               VObjectType::paVariable_decl_assignment, variables);
  return variables;
}

bool isConstantExpression(
    const FileContent* fC, NodeId node,
    const std::unordered_set<std::string_view>& constantParams,
    const std::unordered_set<std::string_view>& variables,
    std::string_view* nonConstantVar = nullptr) {
  if (!node) return true;

  VObjectType type = fC->Type(node);

  if (type == VObjectType::paConstant_expression ||
      type == VObjectType::paConstant_primary ||
      type == VObjectType::paConstant_mintypmax_expression ||
      type == VObjectType::paConstant_param_expression) {
    return true;
  }

  if (type == VObjectType::slIntConst || type == VObjectType::slRealConst ||
      type == VObjectType::paNumber_TickB0) {
    return true;
  }

  if (type >= VObjectType::paNumber_1Tickb0 &&
      type <= VObjectType::paNumber_1TickB1) {
    return true;
  }

  if (type == VObjectType::slStringConst) {
    std::string_view name = fC->SymName(node);

    if (variables.contains(name)) {
      if (nonConstantVar) {
        *nonConstantVar = name;
      }
      return false;
    }

    if (constantParams.contains(name)) {
      return true;
    }

    return true;
  }

  if (type == VObjectType::paPrimary_literal) {
    NodeId child = fC->Child(node);
    return isConstantExpression(fC, child, constantParams, variables,
                                nonConstantVar);
  }

  if (type == VObjectType::paPrimary) {
    NodeId child = fC->Child(node);
    return isConstantExpression(fC, child, constantParams, variables,
                                nonConstantVar);
  }

  if (type == VObjectType::paHierarchical_identifier ||
      type == VObjectType::paPs_or_hierarchical_identifier) {
    NodeId child = fC->Child(node);
    return isConstantExpression(fC, child, constantParams, variables,
                                nonConstantVar);
  }

  if (type == VObjectType::paExpression) {
    for (NodeId child = fC->Child(node); child; child = fC->Sibling(child)) {
      VObjectType childType = fC->Type(child);

      if (childType >= VObjectType::paBinOp_Plus &&
          childType <= VObjectType::paEdge_descriptor) {
        continue;
      }

      if (childType >= VObjectType::paUnary_Minus &&
          childType <= VObjectType::paUnary_Tilda) {
        continue;
      }

      if (!isConstantExpression(fC, child, constantParams, variables,
                                nonConstantVar)) {
        return false;
      }
    }
    return true;
  }

  if (type == VObjectType::paConstant_range) {
    return true;
  }

  if (type == VObjectType::paMintypmax_expression) {
    for (NodeId child = fC->Child(node); child; child = fC->Sibling(child)) {
      if (!isConstantExpression(fC, child, constantParams, variables,
                                nonConstantVar)) {
        return false;
      }
    }
    return true;
  }

  for (NodeId child = fC->Child(node); child; child = fC->Sibling(child)) {
    if (!isConstantExpression(fC, child, constantParams, variables,
                              nonConstantVar)) {
      return false;
    }
  }

  return true;
}

void checkSingleMultipleConcatenation(
    const FileContent* fC, NodeId multiConcatNode,
    const std::unordered_set<std::string_view>& constantParams,
    const std::unordered_set<std::string_view>& variables,
    ErrorContainer* errors, SymbolTable* symbols) {
  if (!multiConcatNode) return;

  NodeId multiplierExpr = fC->Child(multiConcatNode);
  if (!multiplierExpr) return;

  std::string_view nonConstantVar;
  if (!isConstantExpression(fC, multiplierExpr, constantParams, variables,
                            &nonConstantVar)) {
    reportError(fC, multiplierExpr, nonConstantVar,
                ErrorDefinition::LINT_CONCATENATION_MULTIPLIER, errors,
                symbols);
  }
}

void checkConcatenationMultiplier(const FileContent* fC, ErrorContainer* errors,
                                  SymbolTable* symbols) {
  if (!fC || !errors || !symbols) return;

  NodeId root = fC->getRootNode();
  if (!root) return;

  std::unordered_set<std::string_view> constantParams =
      collectConstantParameters(fC);
  std::unordered_set<std::string_view> variables = collectVariables(fC);

  for (NodeId node :
       fC->sl_collect_all(root, VObjectType::paMultiple_concatenation)) {
    checkSingleMultipleConcatenation(fC, node, constantParams, variables,
                                     errors, symbols);
  }
}
