#include "rules/parameter_dynamic_array.h"

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

std::string findParamName(const FileContent* fC, NodeId paramDeclId) {
  return extractParameterName(fC, paramDeclId);
}

void checkParameterDynamicArray(const FileContent* fC, ErrorContainer* errors,
                                SymbolTable* symbols) {
  NodeId root = fC->getRootNode();

  std::vector<VObjectType> declTypes = {
      VObjectType::paParameter_declaration,
      VObjectType::paLocal_parameter_declaration};

  for (auto declType : declTypes) {
    auto paramDecls = fC->sl_collect_all(root, declType);

    for (NodeId decl : paramDecls) {
      std::string paramName = findParamName(fC, decl);

      auto unsizedDims =
          fC->sl_collect_all(decl, VObjectType::paUnsized_dimension);

      if (!unsizedDims.empty()) {
        NodeId errNode = unsizedDims[0];
        reportError(fC, errNode, paramName,
                    ErrorDefinition::LINT_PARAMETR_DYNAMIC_ARRAY, errors,
                    symbols);
      }
    }
  }
}

