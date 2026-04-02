#include "rules/coverpoint_expression_type.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/Design.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <algorithm>
#include <array>
#include <string_view>

#include "main/lint_rules.h"
#include "utils/location_utils.h"
#include "utils/name_utils.h"

namespace SL = SURELOG;

namespace {
auto IsIntegralType(SL::VObjectType type) -> bool {
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

auto ResolveVarName(const SL::FileContent* fileContent, SL::NodeId exprNode)
    -> std::string_view {
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

auto TypeFromVarDecl(const SL::FileContent* fileContent,
                     std::string_view varName) -> SL::VObjectType {
  for (SL::NodeId const kVarDeclId :
       fileContent->sl_collect_all(fileContent->getRootNode(),
                                   SL::VObjectType::paVariable_declaration)) {
    for (SL::NodeId const kAssignId : fileContent->sl_collect_all(
             kVarDeclId, SL::VObjectType::paVariable_decl_assignment, false)) {
      SL::NodeId const kNameNode = fileContent->Child(kAssignId);
      if (kNameNode == SL::InvalidNodeId) {
        continue;
      }
      if (fileContent->SymName(kNameNode) != varName) {
        continue;
      }

      SL::NodeId const kTypeNode = fileContent->Child(kVarDeclId);
      if (kTypeNode == SL::InvalidNodeId) {
        continue;
      }
      SL::NodeId const kBaseTypeNode = fileContent->Child(kTypeNode);
      if (kBaseTypeNode == SL::InvalidNodeId) {
        continue;
      }
      return fileContent->Type(kBaseTypeNode);
    }
  }
  return SL::VObjectType::slNoType;
}

auto TypeFromPortDecl(const SL::FileContent* fileContent,
                      std::string_view varName) -> SL::VObjectType {
  for (SL::NodeId const kPortDeclId :
       fileContent->sl_collect_all(fileContent->getRootNode(),
                                   SL::VObjectType::paAnsi_port_declaration)) {
    SL::NodeId const kHeader = fileContent->Child(kPortDeclId);
    SL::NodeId const kNameNode = (kHeader != SL::InvalidNodeId)
                                     ? fileContent->Sibling(kHeader)
                                     : SL::InvalidNodeId;

    if (kNameNode == SL::InvalidNodeId ||
        fileContent->Type(kNameNode) != SL::VObjectType::slStringConst ||
        fileContent->SymName(kNameNode) != varName) {
      continue;
    }

    SL::NodeId const kPortDir = fileContent->Child(kHeader);
    SL::NodeId const kNetType = (kPortDir != SL::InvalidNodeId)
                                    ? fileContent->Sibling(kPortDir)
                                    : SL::InvalidNodeId;
    SL::NodeId const kDataType =
        (kNetType != SL::InvalidNodeId)
            ? fileContent->Child(fileContent->Child(kNetType))
            : SL::InvalidNodeId;
    SL::NodeId const kBase = (kDataType != SL::InvalidNodeId)
                                 ? fileContent->Child(kDataType)
                                 : kDataType;
    if (kBase != SL::InvalidNodeId) {
      return fileContent->Type(kBase);
    }
  }
  return SL::VObjectType::slNoType;
}

auto TypeFromTfPortItem(const SL::FileContent* fileContent,
                        std::string_view varName) -> SL::VObjectType {
  for (SL::NodeId const kTfId : fileContent->sl_collect_all(
           fileContent->getRootNode(), SL::VObjectType::paTf_port_item)) {
    SL::NodeId nameNode = fileContent->Child(kTfId);
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

    SL::NodeId const kDtNode = fileContent->Child(kTfId);
    if (kDtNode == SL::InvalidNodeId) {
      continue;
    }
    SL::NodeId const kDataType = fileContent->Child(kDtNode);
    if (kDataType == SL::InvalidNodeId) {
      continue;
    }
    SL::NodeId const kBase = fileContent->Child(kDataType);
    if (kBase != SL::InvalidNodeId) {
      return fileContent->Type(kBase);
    } else {
      return fileContent->Type(kDataType);
    }
  }
  return SL::VObjectType::slNoType;
}

auto GetVariableType(const SL::FileContent* fileContent, SL::NodeId exprNode)
    -> SL::VObjectType {
  std::string_view const kVarName = ResolveVarName(fileContent, exprNode);
  if (kVarName.empty()) {
    return SL::VObjectType::slNoType;
  }

  SL::VObjectType result = TypeFromVarDecl(fileContent, kVarName);
  if (result != SL::VObjectType::slNoType) {
    return result;
  }

  result = TypeFromPortDecl(fileContent, kVarName);
  if (result != SL::VObjectType::slNoType) {
    return result;
  }

  return TypeFromTfPortItem(fileContent, kVarName);
}

void CheckSingleCoverpoint(const SL::FileContent* fileContent, SL::NodeId cpId,
                           SL::ErrorContainer* errors,
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

  SL::VObjectType const kVarType = GetVariableType(fileContent, exprNode);
  if (!IsIntegralType(kVarType)) {
    std::string_view const kCpName = ExtractName(fileContent, cpId);
    ReportError(fileContent, cpId, kCpName,
                verihogg_lint::LINT_COVERPOINT_EXPRESSION_TYPE, errors,
                symbols);
  }
}
}  // namespace

void CheckCoverpointExpressionType(const SL::FileContent* fileContent,
                                   SL::ErrorContainer* errors,
                                   SL::SymbolTable* symbols) {
  if (fileContent == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }
  SL::NodeId const kRoot = fileContent->getRootNode();
  if (!kRoot) {
    return;
  }

  auto coverpoints =
      fileContent->sl_collect_all(kRoot, SL::VObjectType::paCover_point);

  for (SL::NodeId const kCpId : coverpoints) {
    CheckSingleCoverpoint(fileContent, kCpId, errors, symbols);
  }
}