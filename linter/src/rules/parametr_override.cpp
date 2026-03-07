#include <algorithm>
#include <array>

#include "Surelog/Design/FileContent.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"
#include "Surelog/SourceCompile/VObjectTypes.h"
#include "rules/parameter_override.h"
#include "utils/location_utils.h"
#include "utils/name_utils.h"

using namespace SURELOG;

static constexpr std::array kLiteralTypes = {
    VObjectType::slIntConst,
    VObjectType::slRealConst,
    VObjectType::slStringConst,
    VObjectType::ppNumber,
};

static constexpr std::array kConstantTypes = {
    VObjectType::paConstant_expression,
    VObjectType::paPrimary_literal,
    VObjectType::paConstant_primary,
};

static constexpr std::array kInstanceTypes = {
    VObjectType::paHierarchical_instance,
    VObjectType::paName_of_instance,
};

static bool isParameterOverrideValid(const FileContent* fC, NodeId instNode) {
  if (!fC || !instNode) return true;

  NodeId child = fC->Child(instNode);
  if (!child) return true;

  NodeId secondChild = fC->Sibling(child);
  if (!secondChild) return true;

  VObjectType secondType = fC->Type(secondChild);

  if (std::ranges::any_of(kLiteralTypes, [secondType](VObjectType t) {
        return t == secondType;
      }))
    return false;

  if (std::ranges::any_of(kConstantTypes, [secondType](VObjectType t) {
        return t == secondType;
      })) {
    NodeId thirdChild = fC->Sibling(secondChild);
    if (thirdChild) {
      VObjectType thirdType = fC->Type(thirdChild);
      if (std::ranges::any_of(kInstanceTypes, [thirdType](VObjectType t) {
            return t == thirdType;
          }))
        return false;
    }
  }

  return true;
}

void checkParameterOverride(const FileContent* fC, ErrorContainer* errors,
                            SymbolTable* symbols) {
  if (!fC || !errors || !symbols) return;

  NodeId root = fC->getRootNode();
  if (!root) return;

  for (NodeId inst :
       fC->sl_collect_all(root, VObjectType::paModule_instantiation)) {
    if (isParameterOverrideValid(fC, inst)) continue;

    NodeId moduleName = fC->Child(inst);
    NodeId badNode = moduleName ? fC->Sibling(moduleName) : NodeId{};
    if (!badNode) badNode = inst;

    reportError(fC, badNode, extractName(fC, badNode),
                ErrorDefinition::LINT_PARAMETR_OVERRIDE, errors, symbols);
  }
}