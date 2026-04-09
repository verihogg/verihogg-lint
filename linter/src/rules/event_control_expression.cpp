#include "rules/event_control_expression.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <string_view>

#include "main/lint_rules.h"
#include "utils/ast_utils.h"
#include "utils/location_utils.h"
#include "utils/name_utils.h"

namespace SL = SURELOG;

namespace {

auto GetLeafExpression(const SL::FileContent* fc, SL::NodeId eventExprId)
    -> SL::NodeId {
  if (fc == nullptr || !eventExprId) {
    return SL::InvalidNodeId;
  }

  SL::NodeId child = fc->Child(eventExprId);
  if (!child) {
    return SL::InvalidNodeId;
  }

  if (fc->Type(child) == SL::VObjectType::paEdge_Posedge ||
      fc->Type(child) == SL::VObjectType::paEdge_Negedge) {
    child = fc->Sibling(child);
  }

  if (!child || fc->Type(child) != SL::VObjectType::paExpression) {
    return SL::InvalidNodeId;
  }

  return child;
}

auto IsBareIdentifier(const SL::FileContent* fc, SL::NodeId eventExprId)
    -> bool {
  SL::NodeId const expr = GetLeafExpression(fc, eventExprId);
  if (!expr) {
    return false;
  }

  SL::NodeId const primary = fc->Child(expr);
  if (!primary || fc->Type(primary) != SL::VObjectType::paPrimary) {
    return false;
  }

  SL::NodeId const primaryChild = fc->Child(primary);
  return primaryChild &&
         fc->Type(primaryChild) == SL::VObjectType::paPrimary_literal;
}

auto GetIdentifierName(const SL::FileContent* fc, SL::NodeId eventExprId)
    -> std::string_view {
  SL::NodeId const expr = GetLeafExpression(fc, eventExprId);
  if (!expr) {
    return "";
  }

  SL::NodeId const primary = fc->Child(expr);
  if (!primary) {
    return "";
  }

  SL::NodeId const primaryLit = fc->Child(primary);
  if (!primaryLit) {
    return "";
  }

  SL::NodeId const strConst = fc->Child(primaryLit);
  if (!strConst || fc->Type(strConst) != SL::VObjectType::slStringConst) {
    return "";
  }

  return fc->SymName(strConst);
}

auto IsEventVariable(const SL::FileContent* fc, SL::NodeId declNode) -> bool {
  SL::NodeId const dataType = GetDataType(fc, declNode);
  if (!dataType) {
    return false;
  }

  SL::NodeId const child = fc->Child(dataType);
  return child && fc->Type(child) == SL::VObjectType::paEvent_type;
}

auto IsVirtualInterface(const SL::FileContent* fc, SL::NodeId declNode)
    -> bool {
  SL::NodeId const dataType = GetDataType(fc, declNode);
  if (!dataType) {
    return false;
  }

  return !fc->sl_collect_all(dataType, SL::VObjectType::paInterface_identifier)
              .empty();
}

auto IsStructTypedef(const SL::FileContent* fc, SL::NodeId root,
                     std::string_view typeName) -> bool {
  if (fc == nullptr || !root || typeName.empty()) {
    return false;
  }

  for (SL::NodeId const typeDecl :
       fc->sl_collect_all(root, SL::VObjectType::paType_declaration)) {
    SL::NodeId dataType = SL::InvalidNodeId;
    std::string_view declaredName;

    for (SL::NodeId cur = fc->Child(typeDecl); cur; cur = fc->Sibling(cur)) {
      SL::VObjectType const t = fc->Type(cur);
      if (t == SL::VObjectType::paData_type) {
        dataType = cur;
      } else if (t == SL::VObjectType::slStringConst && dataType) {
        declaredName = fc->SymName(cur);
      }
    }

    if (declaredName != typeName || !dataType) {
      continue;
    }

    if (!fc->sl_collect_all(dataType, SL::VObjectType::paStruct_union)
             .empty()) {
      return true;
    }
  }

  return false;
}

auto IsNonSingularType(const SL::FileContent* fc, SL::NodeId root,
                       std::string_view varName) -> bool {
  if (IsModuleOrInterfaceInstance(fc, root, varName)) {
    return true;
  }

  SL::NodeId const declNode = FindVarOrNetDecl(fc, root, varName);
  if (!declNode) {
    return false;
  }

  if (IsEventVariable(fc, declNode)) {
    return false;
  }

  if (IsVirtualInterface(fc, declNode)) {
    return true;
  }

  if (HasUnpackedDimension(fc, declNode)) {
    return true;
  }

  std::string_view const typeName = GetTypedefName(fc, declNode);
  if (!typeName.empty() && IsStructTypedef(fc, root, typeName)) {
    return true;
  }

  return false;
}

}  // namespace

void CheckEventControlExpression(const SL::FileContent* fileContent,
                                 SL::ErrorContainer* errors,
                                 SL::SymbolTable* symbols) {
  if (fileContent == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }

  SL::NodeId const root = fileContent->getRootNode();
  if (!root) {
    return;
  }

  for (SL::NodeId const eventCtrlId :
       fileContent->sl_collect_all(root, SL::VObjectType::paEvent_control)) {
    for (SL::NodeId const eventExprId : fileContent->sl_collect_all(
             eventCtrlId, SL::VObjectType::paEvent_expression)) {
      if (!IsBareIdentifier(fileContent, eventExprId)) {
        continue;
      }

      std::string_view const varName =
          GetIdentifierName(fileContent, eventExprId);
      if (varName.empty()) {
        continue;
      }

      if (IsNonSingularType(fileContent, root, varName)) {
        ReportError(fileContent, eventExprId, varName,
                    verihogg_lint::LINT_EVENT_CONTROL_EXPRESSION, errors,
                    symbols);
      }
    }
  }
}