#include "rules/parameter_override.h"

#include <cstdint>
#include <string>

#include "Surelog/Design/Design.h"
#include "Surelog/Design/FileContent.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"
#include "Surelog/SourceCompile/VObjectTypes.h"
#include "utils/name_utils.h"
#include "utils/location_utils.h"

using namespace SURELOG;

bool isParameterOverrideValid(const FileContent* fC, NodeId instNode) {
  if (!fC || !instNode) return true;

  NodeId child = fC->Child(instNode);
  if (!child) return true;

  NodeId secondChild = fC->Sibling(child);
  if (!secondChild) return true;

  VObjectType secondType = fC->Type(secondChild);

  if (secondType == VObjectType::slIntConst ||
      secondType == VObjectType::slRealConst ||
      secondType == VObjectType::slStringConst ||
      secondType == VObjectType::ppNumber) {
    return false;
  }

  if (secondType == VObjectType::paConstant_expression ||
      secondType == VObjectType::paPrimary_literal ||
      secondType == VObjectType::paConstant_primary) {
    NodeId thirdChild = fC->Sibling(secondChild);
    if (thirdChild) {
      VObjectType thirdType = fC->Type(thirdChild);
      if (thirdType == VObjectType::paHierarchical_instance ||
          thirdType == VObjectType::paName_of_instance) {
        return false;
      }
    }
  }

  return true;
}

void reportParameterOverrideError(const FileContent* fC, NodeId badNode,
                                  ErrorContainer* errors,
                                  SymbolTable* symbols) {
  if (!fC || !badNode || !errors || !symbols) return;

  std::string tokenName = extractName(fC, badNode);
  reportError(fC, badNode, tokenName, ErrorDefinition::LINT_PARAMETR_OVERRIDE,
              errors, symbols);
}

void checkParameterOverride(const FileContent* fC, ErrorContainer* errors,
                            SymbolTable* symbols) {
  if (!fC || !errors || !symbols) return;

  NodeId root = fC->getRootNode();
  if (!root) return;

  auto instantiations =
      fC->sl_collect_all(root, VObjectType::paModule_instantiation);

  for (NodeId inst : instantiations) {
    if (!inst) continue;

    if (!isParameterOverrideValid(fC, inst)) {
      NodeId moduleName = fC->Child(inst);
      NodeId badNode = fC->Sibling(moduleName);

      if (!badNode) badNode = inst;

      reportParameterOverrideError(fC, badNode, errors, symbols);
    }
  }
}