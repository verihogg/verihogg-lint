#include "rules/concatenation_multiplier.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/Design.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/ErrorReporting/ErrorDefinition.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <algorithm>
#include <array>
#include <stack>
#include <string_view>
#include <type_traits>
#include <unordered_set>
#include <utility>

#include "utils/location_utils.h"
#include "utils/name_utils.h"

namespace SL = SURELOG;

static constexpr std::array kConstantTypes = {
    SL::VObjectType::paConstant_expression,
    SL::VObjectType::paConstant_primary,
    SL::VObjectType::paConstant_mintypmax_expression,
    SL::VObjectType::paConstant_param_expression,
    SL::VObjectType::paConstant_range,
};

static constexpr std::array kLiteralTypes = {
    SL::VObjectType::slIntConst,
    SL::VObjectType::slRealConst,
    SL::VObjectType::paNumber_TickB0,
};

static constexpr std::array kPassThroughTypes = {
    SL::VObjectType::paPrimary_literal,
    SL::VObjectType::paPrimary,
    SL::VObjectType::paHierarchical_identifier,
    SL::VObjectType::paPs_or_hierarchical_identifier,
};

static constexpr std::array kParamDeclTypes = {
    std::pair{SL::VObjectType::paParameter_declaration,
              SL::VObjectType::paParam_assignment},
    std::pair{SL::VObjectType::paLocal_parameter_declaration,
              SL::VObjectType::paParam_assignment},
};

static constexpr std::array kVarDeclTypes = {
    std::pair{SL::VObjectType::paVariable_declaration,
              SL::VObjectType::paVariable_decl_assignment},
    std::pair{SL::VObjectType::paData_declaration,
              SL::VObjectType::paVariable_decl_assignment},
};

namespace {
auto CollectConstantParameters(const SL::FileContent* fileContent)
    -> std::unordered_set<std::string_view> {
  std::unordered_set<std::string_view> constants;

  SL::NodeId const root = fileContent->getRootNode();
  for (const auto& [parentType, assignType] : kParamDeclTypes) {
    CollectNames(fileContent, root, parentType, assignType, constants);
  }
  return constants;
}

auto CollectVariables(const SL::FileContent* fileContent)
    -> std::unordered_set<std::string_view> {
  std::unordered_set<std::string_view> variables;

  SL::NodeId const root = fileContent->getRootNode();
  for (const auto& [parentType, assignType] : kVarDeclTypes) {
    CollectNames(fileContent, root, parentType, assignType, variables);
  }
  return variables;
}

auto IsOperatorType(SL::VObjectType childType) -> bool {
  using UnderlyingType = std::underlying_type_t<SL::VObjectType>;
  const auto kVal = static_cast<UnderlyingType>(childType);
  return kVal >= static_cast<UnderlyingType>(SL::VObjectType::paUnary_Minus) &&
         kVal <= static_cast<UnderlyingType>(SL::VObjectType::paUnary_Tilda);
}

void EnqueueChildren(const SL::FileContent* fileContent, SL::NodeId node,
                     std::stack<SL::NodeId>& workList) {
  for (SL::NodeId child = fileContent->Child(node); child;
       child = fileContent->Sibling(child)) {
    if (!IsOperatorType(fileContent->Type(child))) {
      workList.push(child);
    }
  }
}

auto CheckIdentifier(std::string_view name,
                     const std::unordered_set<std::string_view>& variables,
                     std::string_view* nonConstantVar) -> bool {
  if (!variables.contains(name)) {
    return true;
  }
  if (nonConstantVar != nullptr) {
    *nonConstantVar = name;
  }
  return false;
}

auto IsConstantExpression(const SL::FileContent* fileContent, SL::NodeId node,
                          const std::unordered_set<std::string_view>& variables,
                          std::string_view* nonConstantVar = nullptr) -> bool {
  if (!node) {
    return true;
  }

  std::stack<SL::NodeId> workList;
  workList.push(node);

  while (!workList.empty()) {
    const SL::NodeId kCurrent = workList.top();
    workList.pop();

    if (!kCurrent) {
      continue;
    }

    const SL::VObjectType kType = fileContent->Type(kCurrent);

    if (std::ranges::find(kConstantTypes, kType) != kConstantTypes.end()) {
      continue;
    }

    if (std::ranges::find(kLiteralTypes, kType) != kLiteralTypes.end()) {
      continue;
    }

    if (std::ranges::find(kPassThroughTypes, kType) !=
        kPassThroughTypes.end()) {
      workList.push(fileContent->Child(kCurrent));
      continue;
    }

    if (kType == SL::VObjectType::slStringConst) {
      if (!CheckIdentifier(fileContent->SymName(kCurrent), variables,
                           nonConstantVar)) {
        return false;
      }
      continue;
    }

    EnqueueChildren(fileContent, kCurrent, workList);
  }

  return true;
}

void CheckSingleMultipleConcatenation(
    const SL::FileContent* fileContent, SL::NodeId multiConcatNode,
    const std::unordered_set<std::string_view>& variables,
    SL::ErrorContainer* errors, SL::SymbolTable* symbols) {
  if (!multiConcatNode) {
    return;
  }

  SL::NodeId const multiplierExpr = fileContent->Child(multiConcatNode);
  if (!multiplierExpr) {
    return;
  }

  std::string_view nonConstantVar;
  if (!IsConstantExpression(fileContent, multiplierExpr, variables,
                            &nonConstantVar)) {
    ReportError(fileContent, multiplierExpr, nonConstantVar,
                SL::ErrorDefinition::LINT_CONCATENATION_MULTIPLIER, errors,
                symbols);
  }
}
}  // namespace

void CheckConcatenationMultiplier(const SL::FileContent* fileContent,
                                  SL::ErrorContainer* errors,
                                  SL::SymbolTable* symbols) {
  if (fileContent == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }

  SL::NodeId const root = fileContent->getRootNode();
  if (!root) {
    return;
  }

  auto constantParams = CollectConstantParameters(fileContent);
  auto variables = CollectVariables(fileContent);

  for (SL::NodeId const node : fileContent->sl_collect_all(
           root, SL::VObjectType::paMultiple_concatenation)) {
    CheckSingleMultipleConcatenation(fileContent, node, variables, errors,
                                     symbols);
  }
}