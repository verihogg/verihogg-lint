#include "rules/coverpoint_expression_type.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/Design.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

#include "main/lint_rules.h"
#include "utils/ast_utils.h"
#include "utils/class_scope_utils.h"
#include "utils/design_utils.h"
#include "utils/location_utils.h"
#include "utils/name_utils.h"

namespace SL = SURELOG;

namespace {

using FieldTypeMap =
    std::unordered_map<std::string,
                       std::unordered_map<std::string, SL::VObjectType>>;

auto IsTypeNameIntegral(const SL::FileContent* fc, std::string_view typeName,
                        int depth = 0) -> std::optional<bool> {
  if (depth > 4 || typeName.empty()) {
    return std::nullopt;
  }

  for (SL::NodeId const kDecl : fc->sl_collect_all(
           fc->getRootNode(), SL::VObjectType::paType_declaration)) {
    SL::NodeId dtNode = SL::InvalidNodeId;
    bool seenDt = false;
    for (SL::NodeId cur = fc->Child(kDecl); cur; cur = fc->Sibling(cur)) {
      SL::VObjectType const kT = fc->Type(cur);
      if (kT == SL::VObjectType::paData_type) {
        dtNode = cur;
        seenDt = true;
      } else if (seenDt && kT == SL::VObjectType::slStringConst) {
        if (fc->SymName(cur) != typeName) {
          break;
        }
        if (!dtNode) {
          return std::nullopt;
        }
        SL::NodeId const kBase = fc->Child(dtNode);
        if (!kBase) {
          return std::nullopt;
        }
        SL::VObjectType const kBaseType = fc->Type(kBase);
        if (IsIntegralType(kBaseType)) {
          return true;
        }
        if (kBaseType == SL::VObjectType::paEnum_base_type) {
          return true;
        }
        if (kBaseType == SL::VObjectType::slStringConst) {
          return IsTypeNameIntegral(fc, fc->SymName(kBase), depth + 1);
        }
        return false;
      }
    }
  }
  return std::nullopt;
}

auto ResolveTypeNode(const SL::FileContent* fc, SL::NodeId typeNode)
    -> SL::VObjectType {
  if (!typeNode) {
    return SL::VObjectType::slNoType;
  }
  SL::VObjectType const kT = fc->Type(typeNode);
  if (kT == SL::VObjectType::slStringConst) {
    auto const kResolved = IsTypeNameIntegral(fc, fc->SymName(typeNode));
    if (kResolved.has_value()) {
      return *kResolved ? SL::VObjectType::paIntVec_TypeBit
                        : SL::VObjectType::paNonIntType_Real;
    }
  }
  return kT;
}

auto BaseTypeOfVarDecl(const SL::FileContent* fc, SL::NodeId varDecl)
    -> SL::VObjectType {
  SL::NodeId const kDtoI = fc->Child(varDecl);
  if (!kDtoI) {
    return SL::VObjectType::slNoType;
  }
  SL::NodeId const kDt = fc->Child(kDtoI);
  if (!kDt) {
    return SL::VObjectType::slNoType;
  }
  SL::NodeId const kBase = fc->Child(kDt);
  if (!kBase) {
    return SL::VObjectType::slNoType;
  }
  return ResolveTypeNode(fc, kBase);
}

auto ExtractUserTypeName(const SL::FileContent* fc, SL::NodeId baseTypeNode)
    -> std::string_view {
  if (!baseTypeNode) {
    return {};
  }
  SL::VObjectType const kT = fc->Type(baseTypeNode);
  if (kT == SL::VObjectType::slStringConst) {
    return fc->SymName(baseTypeNode);
  }
  if (kT == SL::VObjectType::paClass_type) {
    SL::NodeId const kName = fc->Child(baseTypeNode);
    if (kName && fc->Type(kName) == SL::VObjectType::slStringConst) {
      return fc->SymName(kName);
    }
  }
  return {};
}

auto UserTypeFromTfPort(const SL::FileContent* fc, std::string_view varName)
    -> std::string_view {
  for (SL::NodeId const kTfId :
       fc->sl_collect_all(fc->getRootNode(), SL::VObjectType::paTf_port_item)) {
    SL::NodeId nameNode = fc->Child(kTfId);
    while (nameNode && fc->Type(nameNode) != SL::VObjectType::slStringConst) {
      nameNode = fc->Sibling(nameNode);
    }
    if (!nameNode || fc->SymName(nameNode) != varName) {
      continue;
    }

    SL::NodeId kDtNode = SL::InvalidNodeId;
    for (SL::NodeId child = fc->Child(kTfId); child;
         child = fc->Sibling(child)) {
      if (fc->Type(child) == SL::VObjectType::paData_type_or_implicit) {
        kDtNode = child;
        break;
      }
    }
    if (!kDtNode) {
      continue;
    }
    SL::NodeId const kDt = fc->Child(kDtNode);
    if (!kDt) {
      continue;
    }
    SL::NodeId const kBase = fc->Child(kDt);
    if (!kBase) {
      continue;
    }
    std::string_view const kName = ExtractUserTypeName(fc, kBase);
    if (!kName.empty()) {
      return kName;
    }
  }
  return {};
}

auto UserTypeFromVarDecl(const SL::FileContent* fc, std::string_view varName)
    -> std::string_view {
  for (SL::NodeId const kVarDecl : fc->sl_collect_all(
           fc->getRootNode(), SL::VObjectType::paVariable_declaration)) {
    for (SL::NodeId const kAssign : fc->sl_collect_all(
             kVarDecl, SL::VObjectType::paVariable_decl_assignment, false)) {
      SL::NodeId const kNameNode = fc->Child(kAssign);
      if (!kNameNode || fc->SymName(kNameNode) != varName) {
        continue;
      }

      SL::NodeId const kDtoI = fc->Child(kVarDecl);
      if (!kDtoI) {
        continue;
      }
      SL::NodeId const kDt = fc->Child(kDtoI);
      if (!kDt) {
        continue;
      }
      SL::NodeId const kBase = fc->Child(kDt);
      if (!kBase) {
        continue;
      }
      std::string_view const kName = ExtractUserTypeName(fc, kBase);
      if (!kName.empty()) {
        return kName;
      }
    }
  }
  return {};
}

auto GetVarUserTypeName(const SL::FileContent* fc, std::string_view varName)
    -> std::string_view {
  std::string_view name = UserTypeFromTfPort(fc, varName);
  if (!name.empty()) {
    return name;
  }
  return UserTypeFromVarDecl(fc, varName);
}

auto BuildFieldTypeMap(SL::Design* design) -> FieldTypeMap {
  FieldTypeMap result;
  DesignUtils::ForEachFileContent(design, [&](const SL::FileContent* fc) {
    for (SL::NodeId const kClassDecl : fc->sl_collect_all(
             fc->getRootNode(), SL::VObjectType::paClass_declaration)) {
      std::string_view const kClassName =
          ClassScopeUtils::GetClassNameFromDecl(fc, kClassDecl);
      if (kClassName.empty()) {
        continue;
      }

      auto& fieldMap = result[std::string(kClassName)];

      for (SL::NodeId const kProp :
           fc->sl_collect_all(kClassDecl, SL::VObjectType::paClass_property)) {
        for (SL::NodeId const kVarDecl : fc->sl_collect_all(
                 kProp, SL::VObjectType::paVariable_declaration)) {
          SL::VObjectType const kBaseType = BaseTypeOfVarDecl(fc, kVarDecl);
          if (kBaseType == SL::VObjectType::slNoType) {
            continue;
          }

          for (SL::NodeId const kAssign : fc->sl_collect_all(
                   kVarDecl, SL::VObjectType::paVariable_decl_assignment,
                   false)) {
            SL::NodeId const kNameNode = fc->Child(kAssign);
            if (!kNameNode) {
              continue;
            }
            fieldMap[std::string(fc->SymName(kNameNode))] = kBaseType;
          }
        }
      }
    }
  });
  return result;
}

auto GetSimpleVarType(const SL::FileContent* fc, SL::NodeId exprNode)
    -> SL::VObjectType {
  SL::NodeId idNode = exprNode;
  while (idNode && fc->Type(idNode) != SL::VObjectType::slStringConst) {
    idNode = fc->Child(idNode);
  }
  if (!idNode) {
    return SL::VObjectType::slNoType;
  }
  std::string_view const kVarName = fc->SymName(idNode);

  for (SL::NodeId const kTfId :
       fc->sl_collect_all(fc->getRootNode(), SL::VObjectType::paTf_port_item)) {
    SL::NodeId nameNode = fc->Child(kTfId);
    while (nameNode && fc->Type(nameNode) != SL::VObjectType::slStringConst) {
      nameNode = fc->Sibling(nameNode);
    }
    if (!nameNode || fc->SymName(nameNode) != kVarName) {
      continue;
    }

    SL::NodeId kDtNode = SL::InvalidNodeId;
    for (SL::NodeId child = fc->Child(kTfId); child;
         child = fc->Sibling(child)) {
      if (fc->Type(child) == SL::VObjectType::paData_type_or_implicit) {
        kDtNode = child;
        break;
      }
    }
    if (!kDtNode) {
      continue;
    }
    SL::NodeId const kDt = fc->Child(kDtNode);
    if (!kDt) {
      continue;
    }
    SL::NodeId const kBase = fc->Child(kDt);
    if (kBase) {
      return ResolveTypeNode(fc, kBase);
    }
  }

  for (SL::NodeId const kPortDecl : fc->sl_collect_all(
           fc->getRootNode(), SL::VObjectType::paAnsi_port_declaration)) {
    SL::NodeId const kHeader = fc->Child(kPortDecl);
    SL::NodeId const kNameNode =
        kHeader ? fc->Sibling(kHeader) : SL::InvalidNodeId;
    if (!kNameNode || fc->Type(kNameNode) != SL::VObjectType::slStringConst ||
        fc->SymName(kNameNode) != kVarName) {
      continue;
    }

    SL::NodeId const kPortDir = fc->Child(kHeader);
    SL::NodeId kNetType = kPortDir ? fc->Sibling(kPortDir) : SL::InvalidNodeId;
    if (!kNetType && kPortDir) {
      kNetType = kPortDir;
    }
    SL::NodeId const kDataType =
        kNetType ? fc->Child(fc->Child(kNetType)) : SL::InvalidNodeId;
    SL::NodeId const kBase = kDataType ? fc->Child(kDataType) : kDataType;
    if (kBase) {
      return ResolveTypeNode(fc, kBase);
    }
    if (kDataType) {
      return ResolveTypeNode(fc, kDataType);
    }
  }

  for (SL::NodeId const kVarDecl : fc->sl_collect_all(
           fc->getRootNode(), SL::VObjectType::paVariable_declaration)) {
    for (SL::NodeId const kAssign : fc->sl_collect_all(
             kVarDecl, SL::VObjectType::paVariable_decl_assignment, false)) {
      SL::NodeId const kNameNode = fc->Child(kAssign);
      if (!kNameNode || fc->SymName(kNameNode) != kVarName) {
        continue;
      }

      SL::NodeId const kTypeNode = fc->Child(kVarDecl);
      if (!kTypeNode) {
        continue;
      }
      SL::NodeId const kBaseTypeNode = fc->Child(kTypeNode);
      if (kBaseTypeNode) {
        return ResolveTypeNode(fc, kBaseTypeNode);
      }
    }
  }

  return SL::VObjectType::slNoType;
}

void CheckSingleCoverpoint(const SL::FileContent* fc, SL::NodeId cpId,
                           const FieldTypeMap& fieldMap,
                           SL::ErrorContainer* errors,
                           SL::SymbolTable* symbols) {
  SL::NodeId exprNode = SL::InvalidNodeId;
  for (SL::NodeId child = fc->Child(cpId); child; child = fc->Sibling(child)) {
    if (fc->Type(child) == SL::VObjectType::paIFF) {
      break;
    }
    if (fc->Type(child) == SL::VObjectType::paPrimary ||
        fc->Type(child) == SL::VObjectType::paExpression) {
      exprNode = child;
      break;
    }
  }
  if (!exprNode) {
    return;
  }

  auto const kIdents =
      fc->sl_collect_all(exprNode, SL::VObjectType::slStringConst);

  if (kIdents.size() == 1) {
    SL::VObjectType const kVarType = GetSimpleVarType(fc, exprNode);
    if (kVarType == SL::VObjectType::slNoType) {
      return;
    }
    if (kVarType == SL::VObjectType::paClass_type ||
        kVarType == SL::VObjectType::slStringConst) {
      return;
    }
    if (!IsIntegralType(kVarType)) {
      ReportError(fc, cpId, ExtractName(fc, cpId),
                  verihogg_lint::LINT_COVERPOINT_EXPRESSION_TYPE, errors,
                  symbols);
    }
    return;
  }

  if (kIdents.size() == 2) {
    std::string_view const kObjName = fc->SymName(kIdents.at(0));
    std::string_view const kFieldName = fc->SymName(kIdents.at(1));

    std::string_view const kTypeName = GetVarUserTypeName(fc, kObjName);
    if (kTypeName.empty()) {
      return;
    }

    auto const kClassIt = fieldMap.find(std::string(kTypeName));
    if (kClassIt == fieldMap.end()) {
      return;
    }

    auto const kFieldIt = kClassIt->second.find(std::string(kFieldName));
    if (kFieldIt == kClassIt->second.end()) {
      return;
    }

    SL::VObjectType const kFieldType = kFieldIt->second;
    if (kFieldType == SL::VObjectType::slNoType) {
      return;
    }
    if (kFieldType == SL::VObjectType::paClass_type ||
        kFieldType == SL::VObjectType::slStringConst) {
      return;
    }
    if (!IsIntegralType(kFieldType)) {
      ReportError(fc, cpId, ExtractName(fc, cpId),
                  verihogg_lint::LINT_COVERPOINT_EXPRESSION_TYPE, errors,
                  symbols);
    }
    return;
  }
}

}  // namespace

void CheckCoverpointExpressionType(SL::Design* design,
                                   SL::ErrorContainer* errors,
                                   SL::SymbolTable* symbols) {
  if (!design || !errors || !symbols) {
    return;
  }

  FieldTypeMap const kFieldMap = BuildFieldTypeMap(design);

  DesignUtils::ForEachFileContent(design, [&](const SL::FileContent* fc) {
    if (!fc) {
      return;
    }
    SL::NodeId const kRoot = fc->getRootNode();
    if (!kRoot) {
      return;
    }

    for (SL::NodeId const kCpId :
         fc->sl_collect_all(kRoot, SL::VObjectType::paCover_point)) {
      CheckSingleCoverpoint(fc, kCpId, kFieldMap, errors, symbols);
    }
  });
}
