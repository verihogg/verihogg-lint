#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <algorithm>
#include <array>

#include "main/lint_rules.h"
#include "rules/parameter_override.h"
#include "utils/location_utils.h"
#include "utils/name_utils.h"

namespace SL = SURELOG;

static constexpr std::array kLiteralTypes = {
    SL::VObjectType::slIntConst,
    SL::VObjectType::slRealConst,
    SL::VObjectType::slStringConst,
    SL::VObjectType::ppNumber,
};

static constexpr std::array kConstantTypes = {
    SL::VObjectType::paConstant_expression,
    SL::VObjectType::paPrimary_literal,
    SL::VObjectType::paConstant_primary,
};

static constexpr std::array kInstanceTypes = {
    SL::VObjectType::paHierarchical_instance,
    SL::VObjectType::paName_of_instance,
};

namespace {
auto IsParameterOverrideValid(const SL::FileContent* fileContent,
                              SL::NodeId instNode) -> bool {
  if (fileContent == nullptr || !instNode) {
    return true;
  }

  SL::NodeId const kChild = fileContent->Child(instNode);
  if (!kChild) {
    return true;
  }

  SL::NodeId const kSecondChild = fileContent->Sibling(kChild);
  if (!kSecondChild) {
    return true;
  }

  SL::VObjectType const kSecondType = fileContent->Type(kSecondChild);

  if (std::ranges::any_of(kLiteralTypes, [kSecondType](SL::VObjectType type) {
        return type == kSecondType;
      })) {
    return false;
  }

  if (std::ranges::any_of(kConstantTypes, [kSecondType](SL::VObjectType type) {
        return type == kSecondType;
      })) {
    SL::NodeId const kThirdChild = fileContent->Sibling(kSecondChild);
    if (kThirdChild) {
      SL::VObjectType const kThirdType = fileContent->Type(kThirdChild);
      if (std::ranges::any_of(kInstanceTypes,
                              [kThirdType](SL::VObjectType type) {
                                return type == kThirdType;
                              })) {
        return false;
      }
    }
  }

  return true;
}
}  // namespace

void CheckParameterOverride(const SL::FileContent* fileContent,
                            SL::ErrorContainer* errors,
                            SL::SymbolTable* symbols) {
  if (fileContent == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }

  SL::NodeId const kRoot = fileContent->getRootNode();
  if (!kRoot) {
    return;
  }

  for (SL::NodeId const kInst : fileContent->sl_collect_all(
           kRoot, SL::VObjectType::paModule_instantiation)) {
    if (IsParameterOverrideValid(fileContent, kInst)) {
      continue;
    }

    SL::NodeId const kModuleName = fileContent->Child(kInst);
    SL::NodeId badNode =
        kModuleName ? fileContent->Sibling(kModuleName) : SL::NodeId{};
    if (!badNode) {
      badNode = kInst;
    }

    ReportError(fileContent, badNode, ExtractName(fileContent, badNode),
                verihogg_lint::LINT_PARAMETR_OVERRIDE, errors, symbols);
  }
}