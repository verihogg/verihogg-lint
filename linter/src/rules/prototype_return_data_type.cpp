#include "rules/prototype_return_data_type.h"

#include <vector>

#include "Surelog/Design/FileContent.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"
#include "Surelog/SourceCompile/VObjectTypes.h"
#include "utils/location_utils.h"
#include "utils/name_utils.h"

using namespace SURELOG;

static bool HasReturnType(const FileContent* fC, NodeId typeNode) {
  return !fC->sl_collect_all(typeNode, VObjectType::paFunction_data_type, false)
              .empty();
}

void CheckFunctionPrototype(const FileContent* fC, NodeId protoId,
                            ErrorContainer* errors, SymbolTable* symbols) {
  auto ftypeNodes = fC->sl_collect_all(
      protoId, VObjectType::paFunction_data_type_or_implicit, false);
  if (ftypeNodes.empty()) return;

  NodeId typeNode = ftypeNodes.front();
  if (!HasReturnType(fC, typeNode)) {
    ReportError(fC, typeNode, ExtractName(fC, typeNode),
                ErrorDefinition::LINT_PROTOTYPE_RETURN_DATA_TYPE, errors,
                symbols);
  }
}

static std::vector<NodeId> CollectPrototypes(const FileContent* fC,
                                             NodeId parentNode,
                                             VObjectType childType) {
  std::vector<NodeId> result;
  for (NodeId item : fC->sl_collect_all(parentNode, childType)) {
    for (NodeId proto :
         fC->sl_collect_all(item, VObjectType::paFunction_prototype, false))
      result.push_back(proto);
  }
  return result;
}

void CheckPrototypeReturnDataType(const FileContent* fC, ErrorContainer* errors,
                                  SymbolTable* symbols) {
  if (!fC || !errors || !symbols) return;

  NodeId root = fC->getRootNode();
  if (!root) return;

  for (NodeId classId :
       fC->sl_collect_all(root, VObjectType::paClass_declaration)) {
    for (NodeId protoId :
         CollectPrototypes(fC, classId, VObjectType::paClass_method)) {
      CheckFunctionPrototype(fC, protoId, errors, symbols);
    }
  }

  for (NodeId ifaceId :
       fC->sl_collect_all(root, VObjectType::paInterface_declaration)) {
    for (NodeId protoId :
         CollectPrototypes(fC, ifaceId, VObjectType::paNon_port_interface_item))
      CheckFunctionPrototype(fC, protoId, errors, symbols);
  }
}
