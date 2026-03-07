#include "rules/inside_operator.h"

#include <algorithm>
#include <array>
#include <string_view>

#include "Surelog/Design/FileContent.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"
#include "Surelog/SourceCompile/VObjectTypes.h"
#include "utils/location_utils.h"

using namespace SURELOG;

static constexpr std::array kContextTable = {
    std::pair{VObjectType::paUnpacked_dimension, "array dimension"},
    std::pair{VObjectType::paConstant_param_expression, "parametr value"},
    std::pair{VObjectType::paIf_generate_construct, "generate if condition"},
};

static std::string_view getConstantContextName(const FileContent* fC,
                                               NodeId insideNode) {
  for (NodeId cur = fC->Parent(insideNode); cur; cur = fC->Parent(cur)) {
    VObjectType type = fC->Type(cur);
    auto it = std::ranges::find_if(kContextTable, [type](const auto& entry) {
      return entry.first == type;
    });
    if (it != kContextTable.end()) return it->second;
  }
  return "constatnt expression";
}

void checkInsideOperator(const FileContent* fC, ErrorContainer* errors,
                         SymbolTable* symbols) {
  if (!fC || !errors || !symbols) return;

  NodeId root = fC->getRootNode();
  if (!root) return;

  for (NodeId insideId : fC->sl_collect_all(root, VObjectType::paINSIDE)) {
    NodeId parentId = fC->Parent(insideId);
    if (!parentId) continue;

    if (fC->Type(parentId) == VObjectType::paConstant_expression) {
      std::string_view contextName = getConstantContextName(fC, insideId);
      reportError(fC, insideId, contextName,
                  ErrorDefinition::LINT_INSIDE_OPERATOR, errors, symbols);
    }
  }
}
