#include "rules/prototype_return_data_type.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <vector>

#include "main/lint_rules.h"
#include "utils/location_utils.h"
#include "utils/name_utils.h"

namespace SL = SURELOG;

namespace {
auto HasReturnType(const SL::FileContent* fileContent, SL::NodeId typeNode)
    -> bool {
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

  SL::NodeId const kTypeNode = ftypeNodes.front();
  if (!HasReturnType(fileContent, kTypeNode)) {
    ReportError(fileContent, kTypeNode, ExtractName(fileContent, kTypeNode),
                verihogg_lint::LINT_PROTOTYPE_RETURN_DATA_TYPE, errors,
                symbols);
  }
}

auto CollectPrototypes(const SL::FileContent* fileContent,
                       SL::NodeId parentNode, SL::VObjectType childType)
    -> std::vector<SL::NodeId> {
  std::vector<SL::NodeId> result;
  for (SL::NodeId const kItem :
       fileContent->sl_collect_all(parentNode, childType)) {
    for (SL::NodeId const kProto : fileContent->sl_collect_all(
             kItem, SL::VObjectType::paFunction_prototype, false)) {
      result.push_back(kProto);
    }
  }
  return result;
}
}  // namespace

void CheckPrototypeReturnDataType(const SL::FileContent* fileContent,
                                  SL::ErrorContainer* errors,
                                  SL::SymbolTable* symbols) {
  if (fileContent == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }

  SL::NodeId const kRoot = fileContent->getRootNode();
  if (!kRoot) {
    return;
  }

  for (SL::NodeId const kClassId : fileContent->sl_collect_all(
           kRoot, SL::VObjectType::paClass_declaration)) {
    for (SL::NodeId const kProtoId : CollectPrototypes(
             fileContent, kClassId, SL::VObjectType::paClass_method)) {
      CheckFunctionPrototype(fileContent, kProtoId, errors, symbols);
    }
  }

  for (SL::NodeId const kIfaceId : fileContent->sl_collect_all(
           kRoot, SL::VObjectType::paInterface_declaration)) {
    for (SL::NodeId const kProtoId :
         CollectPrototypes(fileContent, kIfaceId,
                           SL::VObjectType::paNon_port_interface_item)) {
      CheckFunctionPrototype(fileContent, kProtoId, errors, symbols);
    }
  }
}
