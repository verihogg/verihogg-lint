#include "rules/concatenation_multiplier.h"

#include <algorithm>
#include <array>
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

static constexpr std::array kConstantTypes = {
    VObjectType::paConstant_expression,
    VObjectType::paConstant_primary,
    VObjectType::paConstant_mintypmax_expression,
    VObjectType::paConstant_param_expression,
    VObjectType::paConstant_range,
};

static constexpr std::array kLiteralTypes = {
    VObjectType::slIntConst,
    VObjectType::slRealConst,
    VObjectType::paNumber_TickB0,
};

static constexpr std::array kPassThroughTypes = {
    VObjectType::paPrimary_literal,
    VObjectType::paPrimary,
    VObjectType::paHierarchical_identifier,
    VObjectType::paPs_or_hierarchical_identifier,
};

static constexpr std::array kParamDeclTypes = {
    std::pair{VObjectType::paParameter_declaration,
              VObjectType::paParam_assignment},
    std::pair{VObjectType::paLocal_parameter_declaration,
              VObjectType::paParam_assignment},
};

static constexpr std::array kVarDeclTypes = {
    std::pair{VObjectType::paVariable_declaration,
              VObjectType::paVariable_decl_assignment},
    std::pair{VObjectType::paData_declaration,
              VObjectType::paVariable_decl_assignment},
};

std::unordered_set<std::string_view> collectConstantParameters(
    const FileContent* fC) {
  std::unordered_set<std::string_view> constants;

  NodeId root = fC->getRootNode();
  for (const auto& [parentType, assignType] : kParamDeclTypes)
    collectNames(fC, root, parentType, assignType, constants);
  return constants;
}

std::unordered_set<std::string_view> collectVariables(const FileContent* fC) {
  std::unordered_set<std::string_view> variables;

  NodeId root = fC->getRootNode();
  for (const auto& [parentType, assignType] : kVarDeclTypes)
    collectNames(fC, root, parentType, assignType, variables);
  return variables;
}

bool isConstantExpression(
    const FileContent* fC, NodeId node,
    const std::unordered_set<std::string_view>& constantParams,
    const std::unordered_set<std::string_view>& variables,
    std::string_view* nonConstantVar = nullptr) {
  if (!node) return true;

  VObjectType type = fC->Type(node);

  if (std::ranges::find(kConstantTypes, type) != kConstantTypes.end())
    return true;

  if (std::ranges::find(kLiteralTypes, type) != kLiteralTypes.end())
    return true;

  if (type >= VObjectType::paNumber_1Tickb0 &&
      type <= VObjectType::paNumber_1TickB1) {
    return true;
  }

  if (std::ranges::find(kPassThroughTypes, type) != kPassThroughTypes.end())
    return isConstantExpression(fC, fC->Child(node), constantParams, variables,
                                nonConstantVar);

  if (type == VObjectType::slStringConst) {
    std::string_view name = fC->SymName(node);

    if (variables.contains(name)) {
      if (nonConstantVar) {
        *nonConstantVar = name;
      }
      return false;
    }
    return true;
  }

  for (NodeId child = fC->Child(node); child; child = fC->Sibling(child)) {
    VObjectType childType = fC->Type(child);

    if (childType >= VObjectType::paBinOp_Plus &&
        childType <= VObjectType::paEdge_descriptor)
      continue;

    if (childType >= VObjectType::paUnary_Minus &&
        childType <= VObjectType::paUnary_Tilda)
      continue;

    if (!isConstantExpression(fC, child, constantParams, variables,
                              nonConstantVar))
      return false;
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

  auto constantParams = collectConstantParameters(fC);
  auto variables = collectVariables(fC);

  for (NodeId node :
       fC->sl_collect_all(root, VObjectType::paMultiple_concatenation)) {
    checkSingleMultipleConcatenation(fC, node, constantParams, variables,
                                     errors, symbols);
  }
}
