#include "rules/inside_operator.h"

#include <string>

#include "Surelog/Design/FileContent.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"
#include "Surelog/SourceCompile/VObjectTypes.h"
#include "utils/location_utils.h"

using namespace SURELOG;

static std::string getConstantContextName(const FileContent* fC,
                                          NodeId insideNode) {
  NodeId current = fC->Parent(insideNode);
  while (current) {
    VObjectType type = fC->Type(current);

    if (type == VObjectType::paUnpacked_dimension) return "array dimension";
    if (type == VObjectType::paConstant_param_expression)
      return "parameter value";
    if (type == VObjectType::paIf_generate_construct)
      return "generate if condition";

    current = fC->Parent(current);
  }
  return "constant expression";
}

void checkInsideOperator(const FileContent* fC, ErrorContainer* errors,
                         SymbolTable* symbols) {
  if (!fC || !errors || !symbols) return;

  NodeId root = fC->getRootNode();

  auto insideNodes = fC->sl_collect_all(root, VObjectType::paINSIDE);

  for (NodeId insideId : insideNodes) {
    NodeId parentId = fC->Parent(insideId);
    if (!parentId) continue;

    if (fC->Type(parentId) == VObjectType::paConstant_expression) {
      std::string contextName = getConstantContextName(fC, insideId);
      reportError(fC, insideId, contextName,
                  ErrorDefinition::LINT_INSIDE_OPERATOR, errors, symbols);
    }
  }
}
