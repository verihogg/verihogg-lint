#include "rules/parameter_dynamic_array.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <array>

#include "main/lint_rules.h"
#include "utils/location_utils.h"
#include "utils/name_utils.h"

namespace SL = SURELOG;

void CheckParameterDynamicArray(const SL::FileContent* fileContent,
                                SL::ErrorContainer* errors,
                                SL::SymbolTable* symbols) {
  if (fileContent == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }

  SL::NodeId const kRoot = fileContent->getRootNode();
  if (kRoot == SL::InvalidNodeId) {
    return;
  }

  static constexpr std::array kDeclTypes = {
      SL::VObjectType::paParameter_declaration,
      SL::VObjectType::paLocal_parameter_declaration,
  };
  for (auto declType : kDeclTypes) {
    for (SL::NodeId const kDecl :
         fileContent->sl_collect_all(kRoot, declType)) {
      auto unsizedDims = fileContent->sl_collect_all(
          kDecl, SL::VObjectType::paUnsized_dimension);
      if (unsizedDims.empty()) {
        continue;
      }
      {
        ReportError(fileContent, unsizedDims.front(),
                    ExtractParameterName(fileContent, kDecl),
                    verihogg_lint::LINT_PARAMETR_DYNAMIC_ARRAY, errors,
                    symbols);
      }
    }
  }
}
