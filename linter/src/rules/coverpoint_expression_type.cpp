#include "rules/coverpoint_expression_type.h"

#include <Surelog/Design/Design.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <string_view>

#include "utils/location_utils.h"
#include "utils/name_utils.h"

namespace SL = SURELOG;

static auto IsIntegralType(SL::VObjectType type) -> bool {
  static constexpr std::array kIntegralTypes = {
      SL::VObjectType::paIntVec_TypeBit,
      SL::VObjectType::paIntVec_TypeLogic,
      SL::VObjectType::paIntVec_TypeReg,
      SL::VObjectType::paIntegerAtomType_Int,
      SL::VObjectType::paIntegerAtomType_LongInt,
      SL::VObjectType::paIntegerAtomType_Shortint,
      SL::VObjectType::paIntegerAtomType_Byte,
      SL::VObjectType::paIntegerAtomType_Integer,
      SL::VObjectType::paIntegerAtomType_Time,
      SL::VObjectType::paEnum_base_type,
  };
  return std::ranges::find(kIntegralTypes, type) != kIntegralTypes.end();
};

static auto ResolveVarName(const SL::FileContent* fileContent,
                           SL::NodeId exprNode) -> std::string_view {
  if (exprNode == SL::InvalidNodeId) {
    return {};
  }

  SL::NodeId idNode = exprNode;
  while (idNode != SL::InvalidNodeId &&
         fileContent->Type(idNode) != SL::VObjectType::slStringConst) {
    idNode = fileContent->Child(idNode);
  }
  if (idNode == SL::InvalidNodeId) {
    return {};
  }
  return fileContent->SymName(idNode);
}

static auto TypeFromVarDecl(const SL::FileContent* fileContent,
                            std::string_view varName) -> SL::VObjectType {
  for (SL::NodeId varDeclId :
       fileContent->sl_collect_all(fileContent->getRootNode(),
                                   SL::VObjectType::paVariable_declaration)) {
    for (SL::NodeId assignId : fileContent->sl_collect_all(
             varDeclId, SL::VObjectType::paVariable_decl_assignment, false)) {
      SL::NodeId nameNode = fileContent->Child(assignId);
      if (nameNode == SL::InvalidNodeId) {
        continue;
      }
      if (fileContent->SymName(nameNode) != varName) {
        continue;
      }

      SL::NodeId typeNode = fileContent->Child(varDeclId);
      if (typeNode == SL::InvalidNodeId) {
        continue;
      }
      SL::NodeId baseTypeNode = fileContent->Child(typeNode);
      if (baseTypeNode == SL::InvalidNodeId) {
        continue;
      }
      return fileContent->Type(baseTypeNode);
    }
  }
  return SL::VObjectType::slNoType;
}

static auto TypeFromPortDecl(const SL::FileContent* fileContent,
                             std::string_view varName) -> SL::VObjectType {
  for (SL::NodeId portDeclId :
       fileContent->sl_collect_all(fileContent->getRootNode(),
                                   SL::VObjectType::paAnsi_port_declaration)) {
    SL::NodeId header = fileContent->Child(portDeclId);
    SL::NodeId nameNode = (header != SL::InvalidNodeId)
                              ? fileContent->Sibling(header)
                              : SL::InvalidNodeId;

    if (nameNode == SL::InvalidNodeId ||
        fileContent->Type(nameNode) != SL::VObjectType::slStringConst ||
        fileContent->SymName(nameNode) != varName) {
      continue;
    }

    SL::NodeId portDir = fileContent->Child(header);
    SL::NodeId netType = (portDir != SL::InvalidNodeId)
                             ? fileContent->Sibling(portDir)
                             : SL::InvalidNodeId;
    SL::NodeId dataType = (netType != SL::InvalidNodeId)
                              ? fileContent->Child(fileContent->Child(netType))
                              : SL::InvalidNodeId;
    SL::NodeId base = (dataType != SL::InvalidNodeId)
                          ? fileContent->Child(dataType)
                          : SL::InvalidNodeId;
    if (base != SL::InvalidNodeId) {
      return fileContent->Type(base);
    }
  }
  return SL::VObjectType::slNoType;
}

static auto TypeFromTfPortItem(const SL::FileContent* fileContent,
                               std::string_view varName) -> SL::VObjectType {
  for (SL::NodeId tfId : fileContent->sl_collect_all(
           fileContent->getRootNode(), SL::VObjectType::paTf_port_item)) {
    SL::NodeId nameNode = fileContent->Child(tfId);
    while (nameNode != SL::InvalidNodeId &&
           fileContent->Type(nameNode) != SL::VObjectType::slStringConst) {
      nameNode = fileContent->Sibling(nameNode);
    }
    if (nameNode == SL::InvalidNodeId) {
      continue;
    }
    if (fileContent->SymName(nameNode) != varName) {
      continue;
    }

    SL::NodeId dtNode = fileContent->Child(tfId);
    if (dtNode == SL::InvalidNodeId) {
      continue;
    }
    SL::NodeId dataType = fileContent->Child(dtNode);
    if (dataType == SL::InvalidNodeId) {
      continue;
    }
    SL::NodeId base = fileContent->Child(dataType);
    if (base != SL::InvalidNodeId) {
      return fileContent->Type(base);
    }
  }
  return SL::VObjectType::slNoType;
}

static auto GetVariableType(const SL::FileContent* fileContent,
                            SL::NodeId exprNode) -> SL::VObjectType {
  std::string_view varName = ResolveVarName(fileContent, exprNode);
  if (varName.empty()) {
    return SL::VObjectType::slNoType;
  }

  SL::VObjectType result = TypeFromVarDecl(fileContent, varName);
  if (result != SL::VObjectType::slNoType) {
    return result;
  }

  result = TypeFromPortDecl(fileContent, varName);
  if (result != SL::VObjectType::slNoType) {
    return result;
  }

  return TypeFromTfPortItem(fileContent, varName);
}

static void CheckSingleCoverpoint(const SL::FileContent* fileContent,
                                  SL::NodeId cpId, SL::ErrorContainer* errors,
                                  SL::SymbolTable* symbols) {
  SL::NodeId exprNode = SL::InvalidNodeId;
  for (SL::NodeId child = fileContent->Child(cpId); child != SL::InvalidNodeId;
       child = fileContent->Sibling(child)) {
    if (fileContent->Type(child) == SL::VObjectType::paPrimary ||
        fileContent->Type(child) == SL::VObjectType::paExpression) {
      exprNode = child;
      break;
    }
  }
  if (exprNode == SL::InvalidNodeId) {
    return;
  }

  SL::VObjectType varType = GetVariableType(fileContent, exprNode);
  if (!IsIntegralType(varType)) {
    std::string_view cpName = ExtractName(fileContent, cpId);
    ReportError(fileContent, cpId, cpName,
                SL::ErrorDefinition::LINT_COVERPOINT_EXPRESSION_TYPE, errors,
                symbols);
  }
}

void CheckCoverpointExpressionType(const SL::FileContent* fileContent,
                                   SL::ErrorContainer* errors,
                                   SL::SymbolTable* symbols) {
  if (fileContent == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }
  SL::NodeId root = fileContent->getRootNode();
  if (!root) {
    return;
  }

  auto coverpoints =
      fileContent->sl_collect_all(root, SL::VObjectType::paCover_point);

  for (SL::NodeId cpId : coverpoints) {
    CheckSingleCoverpoint(fileContent, cpId, errors, symbols);
  }
}
