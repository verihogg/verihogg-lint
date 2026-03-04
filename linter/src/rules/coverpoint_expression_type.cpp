#include "rules/coverpoint_expression_type.h"

#include <cstdint>
#include <string>

#include "Surelog/Design/Design.h"
#include "Surelog/Design/FileContent.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"
#include "Surelog/SourceCompile/VObjectTypes.h"
#include "utils/name_utils.h"
#include"utils/location_utils.h"

using namespace SURELOG;

std::string getCoverpointName(const FileContent* fC, NodeId cpNode) {
  return extractName(fC, cpNode);
}

bool isIntegralType(VObjectType type) {
  switch (type) {
    case VObjectType::paIntVec_TypeBit:
    case VObjectType::paIntVec_TypeLogic:
    case VObjectType::paIntegerAtomType_Int:
    case VObjectType::paIntegerAtomType_LongInt:
    case VObjectType::paIntegerAtomType_Shortint:
    case VObjectType::paIntegerAtomType_Byte:
    case VObjectType::paEnum_base_type:
      return true;
    default:
      return false;
  }
}

VObjectType getVariableType(const FileContent* fC, NodeId exprNode) {
  if (!exprNode) return VObjectType::slNoType;

  NodeId idNode = exprNode;
  while (idNode && fC->Type(idNode) != VObjectType::slStringConst) {
    idNode = fC->Child(idNode);
  }
  if (!idNode) return VObjectType::slNoType;

  std::string varName = std::string(fC->SymName(idNode));

  auto varDeclNodes = fC->sl_collect_all(fC->getRootNode(),
                                         VObjectType::paVariable_declaration);
  for (NodeId varDeclId : varDeclNodes) {
    auto assignNodes = fC->sl_collect_all(
        varDeclId, VObjectType::paVariable_decl_assignment, false);
    for (NodeId assignId : assignNodes) {
      NodeId nameNode = fC->Child(assignId);
      if (!nameNode) continue;

      std::string declName = std::string(fC->SymName(nameNode));
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
  auto tfItems =
      fC->sl_collect_all(fC->getRootNode(), VObjectType::paTf_port_item);
  for (NodeId tfId : tfItems) {
    NodeId nameNode = fC->Child(tfId);
    while (nameNode && fC->Type(nameNode) != VObjectType::slStringConst)
      nameNode = fC->Sibling(nameNode);

    if (!nameNode) continue;

    if (std::string(fC->SymName(nameNode)) == varName) {
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

void checkSingleCoverpoint(const FileContent* fC, NodeId cpId,
                           ErrorContainer* errors, SymbolTable* symbols) {
  NodeId exprNode;

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
    std::string cpName = getCoverpointName(fC, cpId);
    reportError(fC, cpId, cpName,
                ErrorDefinition::LINT_COVERPOINT_EXPRESSION_TYPE, errors,
                symbols);
  }
}

void checkCoverpointExpressionType(const FileContent* fC,
                                   ErrorContainer* errors,
                                   SymbolTable* symbols) {
  NodeId root = fC->getRootNode();
  auto coverpoints = fC->sl_collect_all(root, VObjectType::paCover_point);

  for (NodeId cpId : coverpoints) {
    checkSingleCoverpoint(fC, cpId, errors, symbols);
  }
}
