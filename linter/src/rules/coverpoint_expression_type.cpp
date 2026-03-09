#include "rules/coverpoint_expression_type.h"

#include <string_view>

#include "Surelog/Design/Design.h"
#include "Surelog/Design/FileContent.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"
#include "Surelog/SourceCompile/VObjectTypes.h"
#include "utils/location_utils.h"
#include "utils/name_utils.h"

using namespace SURELOG;

static bool isIntegralType(VObjectType type) {
  static constexpr std::array kIntegralTypes = {
      VObjectType::paIntVec_TypeBit,
      VObjectType::paIntVec_TypeLogic,
      VObjectType::paIntVec_TypeReg,
      VObjectType::paIntegerAtomType_Int,
      VObjectType::paIntegerAtomType_LongInt,
      VObjectType::paIntegerAtomType_Shortint,
      VObjectType::paIntegerAtomType_Byte,
      VObjectType::paIntegerAtomType_Integer,
      VObjectType::paIntegerAtomType_Time,
      VObjectType::paEnum_base_type,
  };
  return std::find(kIntegralTypes.begin(), kIntegralTypes.end(), type) !=
         kIntegralTypes.end();
};

static VObjectType getVariableType(const FileContent* fC, NodeId exprNode) {
  if (!exprNode) return VObjectType::slNoType;

  NodeId idNode = exprNode;
  while (idNode && fC->Type(idNode) != VObjectType::slStringConst) {
    idNode = fC->Child(idNode);
  }
  if (!idNode) return VObjectType::slNoType;

  std::string_view varName = fC->SymName(idNode);

  for (NodeId varDeclId : fC->sl_collect_all(
           fC->getRootNode(), VObjectType::paVariable_declaration)) {
    for (NodeId assignId : fC->sl_collect_all(
             varDeclId, VObjectType::paVariable_decl_assignment, false)) {
      NodeId nameNode = fC->Child(assignId);
      if (!nameNode) continue;

      std::string_view declName = fC->SymName(nameNode);
      if (declName == varName) {
        NodeId typeNode = fC->Child(varDeclId);
        if (typeNode) {
          NodeId baseTypeNode = fC->Child(typeNode);
          if (baseTypeNode) {
            return fC->Type(baseTypeNode);
          }
        }
      }
    }
  }

  for (NodeId portDeclId : fC->sl_collect_all(
           fC->getRootNode(), VObjectType::paAnsi_port_declaration)) {
    NodeId header = fC->Child(portDeclId);
    NodeId nameNode = header ? fC->Sibling(header) : InvalidNodeId;

    if (!nameNode || fC->Type(nameNode) != VObjectType::slStringConst ||
        fC->SymName(nameNode) != varName)
      continue;

    NodeId portDir = fC->Child(header);
    NodeId netType = portDir ? fC->Sibling(portDir) : InvalidNodeId;
    NodeId dataType = netType ? fC->Child(fC->Child(netType)) : InvalidNodeId;

    if (NodeId base = dataType ? fC->Child(dataType) : InvalidNodeId)
      return fC->Type(base);
  }

  auto tfItems =
      fC->sl_collect_all(fC->getRootNode(), VObjectType::paTf_port_item);
  for (NodeId tfId : tfItems) {
    NodeId nameNode = fC->Child(tfId);
    while (nameNode && fC->Type(nameNode) != VObjectType::slStringConst)
      nameNode = fC->Sibling(nameNode);

    if (!nameNode) continue;

    if (fC->SymName(nameNode) == varName) {
      NodeId dtNode = fC->Child(tfId);
      if (dtNode) {
        NodeId dataType = fC->Child(dtNode);
        if (dataType) {
          NodeId base = fC->Child(dataType);
          if (base) return fC->Type(base);
        }
      }
    }
  }

  return VObjectType::slNoType;
}

static void checkSingleCoverpoint(const FileContent* fC, NodeId cpId,
                                  ErrorContainer* errors,
                                  SymbolTable* symbols) {
  NodeId exprNode = InvalidNodeId;

  for (NodeId child = fC->Child(cpId); child; child = fC->Sibling(child)) {
    if (fC->Type(child) == VObjectType::paPrimary ||
        fC->Type(child) == VObjectType::paExpression) {
      exprNode = child;
      break;
    }
  }
  if (!exprNode) return;

  VObjectType varType = getVariableType(fC, exprNode);

  if (!isIntegralType(varType)) {
    std::string_view cpName = extractName(fC, cpId);
    reportError(fC, cpId, cpName,
                ErrorDefinition::LINT_COVERPOINT_EXPRESSION_TYPE, errors,
                symbols);
  }
}

void checkCoverpointExpressionType(const FileContent* fC,
                                   ErrorContainer* errors,
                                   SymbolTable* symbols) {
  if (!fC || !errors || !symbols) return;
  NodeId root = fC->getRootNode();
  if (!root) return;

  auto coverpoints = fC->sl_collect_all(root, VObjectType::paCover_point);

  for (NodeId cpId : coverpoints) {
    checkSingleCoverpoint(fC, cpId, errors, symbols);
  }
}
