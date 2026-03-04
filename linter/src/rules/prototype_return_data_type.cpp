#include "rules/prototype_return_data_type.h"

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

std::string getFunctionName(const FileContent* fC, NodeId typeNode) {
  return extractName(fC, typeNode);
}

bool hasReturnType(const FileContent* fC, NodeId typeNode) {
  for (NodeId child = fC->Child(typeNode); child; child = fC->Sibling(child)) {
    if (fC->Type(child) == VObjectType::paFunction_data_type) {
      return true;
    }
  }
  return false;
}

void checkFunctionPrototype(const FileContent* fC, NodeId protoId,
                            ErrorContainer* errors, SymbolTable* symbols) {
  auto ftypeNodes = fC->sl_collect_all(
      protoId, VObjectType::paFunction_data_type_or_implicit, false);
  if (ftypeNodes.empty()) return;

  NodeId typeNode = ftypeNodes.front();

  if (!hasReturnType(fC, typeNode)) {
    std::string funcName = getFunctionName(fC, typeNode);
    reportError(fC, typeNode, funcName,
                ErrorDefinition::LINT_PROTOTYPE_RETURN_DATA_TYPE, errors,
                symbols);
  }
}

void checkPrototypeReturnDataType(const FileContent* fC, ErrorContainer* errors,
                                  SymbolTable* symbols) {
  NodeId root = fC->getRootNode();

  auto classNodes = fC->sl_collect_all(root, VObjectType::paClass_declaration);
  for (NodeId classId : classNodes) {
    auto methods = fC->sl_collect_all(classId, VObjectType::paClass_method);
    for (NodeId m : methods) {
      auto protoNodes =
          fC->sl_collect_all(m, VObjectType::paFunction_prototype, false);
      for (NodeId protoId : protoNodes) {
        checkFunctionPrototype(fC, protoId, errors, symbols);
      }
    }
  }

  auto interfaceNodes =
      fC->sl_collect_all(root, VObjectType::paInterface_declaration);
  for (NodeId ifaceId : interfaceNodes) {
    auto items =
        fC->sl_collect_all(ifaceId, VObjectType::paNon_port_interface_item);
    for (NodeId item : items) {
      auto externNodes =
          fC->sl_collect_all(item, VObjectType::paExtern_tf_declaration);
      for (NodeId extId : externNodes) {
        auto protoNodes =
            fC->sl_collect_all(extId, VObjectType::paFunction_prototype, false);
        for (NodeId protoId : protoNodes) {
          checkFunctionPrototype(fC, protoId, errors, symbols);
        }
      }
    }
  }
}
