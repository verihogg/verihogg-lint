#include "rules/inside_operator.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <algorithm>
#include <array>
#include <string_view>
#include <utility>

#include "main/lint_rules.h"
#include "utils/location_utils.h"

namespace SL = SURELOG;

static constexpr std::array kContextTable = {
    std::pair{SL::VObjectType::paUnpacked_dimension, "array dimension"},
    std::pair{SL::VObjectType::paConstant_param_expression, "parameter value"},
    std::pair{SL::VObjectType::paIf_generate_construct,
              "generate if condition"},
};

namespace {
auto GetConstantContextName(const SL::FileContent* fileContent,
                            SL::NodeId insideNode) -> std::string_view {
  for (SL::NodeId cur = fileContent->Parent(insideNode); cur;
       cur = fileContent->Parent(cur)) {
    SL::VObjectType type = fileContent->Type(cur);
    const auto* iter = std::ranges::find_if(
        kContextTable,
        [type](const auto& entry) { return entry.first == type; });
    if (iter != kContextTable.end()) {
      return iter->second;
    }
  }
  return "constant expression";
}
}  // namespace

void CheckInsideOperator(const SL::FileContent* fileContent,
                         SL::ErrorContainer* errors, SL::SymbolTable* symbols) {
  if (fileContent == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }

  SL::NodeId const kRoot = fileContent->getRootNode();
  if (!kRoot) {
    return;
  }

  for (SL::NodeId const kInsideId :
       fileContent->sl_collect_all(kRoot, SL::VObjectType::paINSIDE)) {
    SL::NodeId const kParentId = fileContent->Parent(kInsideId);
    if (!kParentId) {
      continue;
    }

    if (fileContent->Type(kParentId) ==
        SL::VObjectType::paConstant_expression) {
      std::string_view const kContextName =
          GetConstantContextName(fileContent, kInsideId);
      ReportError(fileContent, kInsideId, kContextName,
                  verihogg_lint::LINT_INSIDE_OPERATOR, errors, symbols);
    }
  }
}
