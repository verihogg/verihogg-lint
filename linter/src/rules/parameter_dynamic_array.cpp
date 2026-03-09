#include "rules/parameter_dynamic_array.h"

#include <array>

#include "Surelog/Design/FileContent.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"
#include "Surelog/SourceCompile/VObjectTypes.h"
#include "utils/location_utils.h"
#include "utils/name_utils.h"

using namespace SURELOG;

void CheckParameterDynamicArray(const FileContent* fC, ErrorContainer* errors,
                                SymbolTable* symbols) {
  if (!fC || !errors || !symbols) return;

  NodeId root = fC->getRootNode();
  if (!root) return;

  static constexpr std::array kDeclTypes = {
      VObjectType::paParameter_declaration,
      VObjectType::paLocal_parameter_declaration,
  };
  for (auto declType : kDeclTypes) {
    for (NodeId decl : fC->sl_collect_all(root, declType)) {
      auto unsizedDims =
          fC->sl_collect_all(decl, VObjectType::paUnsized_dimension);
      if (unsizedDims.empty()) continue;
      {
        ReportError(fC, unsizedDims.front(), ExtractParameterName(fC, decl),
                    ErrorDefinition::LINT_PARAMETR_DYNAMIC_ARRAY, errors,
                    symbols);
      }
    }
  }
}
