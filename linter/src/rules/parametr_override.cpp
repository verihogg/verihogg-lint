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

  SL::NodeId const child = fileContent->Child(instNode);
  if (!child) {
    return true;
  }

  SL::NodeId const secondChild = fileContent->Sibling(child);
  if (!secondChild) {
    return true;
  }

  SL::VObjectType const secondType = fileContent->Type(secondChild);

  if (std::ranges::any_of(kLiteralTypes, [secondType](SL::VObjectType type) {
        return type == secondType;
      })) {
    return false;
  }

  if (std::ranges::any_of(kConstantTypes, [secondType](SL::VObjectType type) {
        return type == secondType;
      })) {
    SL::NodeId const thirdChild = fileContent->Sibling(secondChild);
    if (thirdChild) {
      SL::VObjectType const thirdType = fileContent->Type(thirdChild);
      if (std::ranges::any_of(kInstanceTypes,
                              [thirdType](SL::VObjectType type) {
                                return type == thirdType;
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

  SL::NodeId const root = fileContent->getRootNode();
  if (!root) {
    return;
  }

  for (SL::NodeId const inst : fileContent->sl_collect_all(
           root, SL::VObjectType::paModule_instantiation)) {
    if (IsParameterOverrideValid(fileContent, inst)) {
      continue;
    }

    SL::NodeId const moduleName = fileContent->Child(inst);
    SL::NodeId badNode =
        moduleName ? fileContent->Sibling(moduleName) : SL::NodeId{};
    if (!badNode) {
      badNode = inst;
    }

    ReportError(fileContent, badNode, ExtractName(fileContent, badNode),
                verihogg_lint::LINT_PARAMETR_OVERRIDE, errors, symbols);
  }
}