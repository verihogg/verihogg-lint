#include "rules/prototype_return_data_type.h"

#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <vector>

#include "utils/location_utils.h"
#include "utils/name_utils.h"

namespace SL = SURELOG;

static auto HasReturnType(const SL::FileContent* fileContent,
                          SL::NodeId typeNode) -> bool {
  return !fileContent
              ->sl_collect_all(typeNode, SL::VObjectType::paFunction_data_type,
                               false)
              .empty();
}

void CheckFunctionPrototype(const SL::FileContent* fileContent,
                            SL::NodeId protoId, SL::ErrorContainer* errors,
                            SL::SymbolTable* symbols) {
  auto ftypeNodes = fileContent->sl_collect_all(
      protoId, SL::VObjectType::paFunction_data_type_or_implicit, false);
  if (ftypeNodes.empty()) {
    return;
  }

  SL::NodeId typeNode = ftypeNodes.front();
  if (!HasReturnType(fileContent, typeNode)) {
    ReportError(fileContent, typeNode, ExtractName(fileContent, typeNode),
                SL::ErrorDefinition::LINT_PROTOTYPE_RETURN_DATA_TYPE, errors,
                symbols);
  }
}

static auto CollectPrototypes(const SL::FileContent* fileContent,
                              SL::NodeId parentNode, SL::VObjectType childType)
    -> std::vector<SL::NodeId> {
  std::vector<SL::NodeId> result;
  for (SL::NodeId item : fileContent->sl_collect_all(parentNode, childType)) {
    for (SL::NodeId proto : fileContent->sl_collect_all(
             item, SL::VObjectType::paFunction_prototype, false)) {
      result.push_back(proto);
    }
  }
  return result;
}

void CheckPrototypeReturnDataType(const SL::FileContent* fileContent,
                                  SL::ErrorContainer* errors,
                                  SL::SymbolTable* symbols) {
  if (fileContent == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }

  SL::NodeId root = fileContent->getRootNode();
  if (!root) {
    return;
  }

  for (SL::NodeId classId : fileContent->sl_collect_all(
           root, SL::VObjectType::paClass_declaration)) {
    for (SL::NodeId protoId : CollectPrototypes(
             fileContent, classId, SL::VObjectType::paClass_method)) {
      CheckFunctionPrototype(fileContent, protoId, errors, symbols);
    }
  }

  for (SL::NodeId ifaceId : fileContent->sl_collect_all(
           root, SL::VObjectType::paInterface_declaration)) {
    for (SL::NodeId protoId :
         CollectPrototypes(fileContent, ifaceId,
                           SL::VObjectType::paNon_port_interface_item)) {
      CheckFunctionPrototype(fileContent, protoId, errors, symbols);
    }
  }
}
