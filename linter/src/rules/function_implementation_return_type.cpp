#include "rules/function_implementation_return_type.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/Design.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/ErrorReporting/ErrorDefinition.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "main/lint_rules.h"
#include "utils/ast_utils.h"
#include "utils/class_scope_utils.h"
#include "utils/design_utils.h"
#include "utils/location_utils.h"

namespace SL = SURELOG;

namespace {

struct ReturnTypeInfo {
  SL::VObjectType nodeType = SL::VObjectType::sl_INVALID_;
  std::string_view name;
};

auto ReturnTypesMatch(const ReturnTypeInfo& proto, const ReturnTypeInfo& impl)
    -> bool {
  if (proto.nodeType != impl.nodeType) {
    return false;
  }
  if (proto.nodeType == SL::VObjectType::slStringConst) {
    return proto.name == impl.name;
  }
  return true;
}

auto ExtractReturnTypeInfo(const SL::FileContent* fc,
                           SL::NodeId func_data_type_or_implicit)
    -> std::optional<ReturnTypeInfo> {
  if (!func_data_type_or_implicit) {
    return std::nullopt;
  }

  SL::NodeId const kFuncDataType = fc->Child(func_data_type_or_implicit);
  if (!kFuncDataType ||
      fc->Type(kFuncDataType) != SL::VObjectType::paFunction_data_type) {
    return std::nullopt;
  }

  SL::NodeId const kDataType = fc->Child(kFuncDataType);
  if (!kDataType || fc->Type(kDataType) != SL::VObjectType::paData_type) {
    return ReturnTypeInfo{
        .nodeType = SL::VObjectType::paFunction_data_type,
        .name = {},
    };
  }

  SL::NodeId const kTypeChild = fc->Child(kDataType);
  if (!kTypeChild) {
    return std::nullopt;
  }

  SL::VObjectType kNodeType = fc->Type(kTypeChild);
  SL::NodeId kNameNode = kTypeChild;

  if (kNodeType == SL::VObjectType::paClass_scope) {
    SL::NodeId const kScopedName = fc->Sibling(kTypeChild);
    if (kScopedName &&
        fc->Type(kScopedName) == SL::VObjectType::slStringConst) {
      kNodeType = SL::VObjectType::slStringConst;
      kNameNode = kScopedName;
    }
  }

  std::string_view name;
  if (kNodeType == SL::VObjectType::slStringConst) {
    name = fc->SymName(kNameNode);
  }

  return ReturnTypeInfo{
      .nodeType = kNodeType,
      .name = name,
  };
}

struct PrototypeInfo {
  std::string_view className;
  std::string_view funcName;
  ReturnTypeInfo returnType;
  SL::NodeId reportNode;
  const SL::FileContent* fileContent;
};

auto MakeKey(std::string_view className, std::string_view funcName)
    -> std::string {
  std::string key;
  key.reserve(className.size() + 2 + funcName.size());
  key.append(className);
  key.append("::");
  key.append(funcName);
  return key;
}

auto GetClassNameFromDecl(const SL::FileContent* fc, SL::NodeId classDecl)
    -> std::string_view {
  SL::NodeId const kClassKw = fc->Child(classDecl);
  if (!kClassKw || fc->Type(kClassKw) != SL::VObjectType::paCLASS) {
    return {};
  }
  SL::NodeId const kNameNode = fc->Sibling(kClassKw);
  if (!kNameNode || fc->Type(kNameNode) != SL::VObjectType::slStringConst) {
    return {};
  }
  return fc->SymName(kNameNode);
}

auto ExtractFuncNameFromPrototype(const SL::FileContent* fc,
                                  SL::NodeId funcProto) -> std::string_view {
  SL::NodeId const kFdtoi = fc->Child(funcProto);
  if (!kFdtoi ||
      fc->Type(kFdtoi) != SL::VObjectType::paFunction_data_type_or_implicit) {
    return {};
  }
  SL::NodeId const kNameNode = fc->Sibling(kFdtoi);
  if (!kNameNode || fc->Type(kNameNode) != SL::VObjectType::slStringConst) {
    return {};
  }
  return fc->SymName(kNameNode);
}

auto TryCollectPrototype(const SL::FileContent* fc, SL::NodeId classItem,
                         std::string_view className)
    -> std::optional<PrototypeInfo> {
  SL::NodeId const kClassMethod = fc->Child(classItem);
  if (!kClassMethod ||
      fc->Type(kClassMethod) != SL::VObjectType::paClass_method) {
    return std::nullopt;
  }

  SL::NodeId const kFirstChild = fc->Child(kClassMethod);
  if (!kFirstChild ||
      fc->Type(kFirstChild) != SL::VObjectType::paExtern_qualifier) {
    return std::nullopt;
  }

  SL::NodeId const kMethodProto =
      FindSiblingOfType(fc, kFirstChild, SL::VObjectType::paMethod_prototype);
  if (!kMethodProto) {
    return std::nullopt;
  }

  SL::NodeId const kProto = fc->Child(kMethodProto);
  if (!kProto || fc->Type(kProto) != SL::VObjectType::paFunction_prototype) {
    return std::nullopt;
  }

  std::string_view const kFuncName = ExtractFuncNameFromPrototype(fc, kProto);
  if (kFuncName.empty()) {
    return std::nullopt;
  }

  SL::NodeId const kFdtoi = fc->Child(kProto);
  if (!kFdtoi ||
      fc->Type(kFdtoi) != SL::VObjectType::paFunction_data_type_or_implicit) {
    return std::nullopt;
  }

  std::optional<ReturnTypeInfo> const kRetType =
      ExtractReturnTypeInfo(fc, kFdtoi);
  if (!kRetType.has_value()) {
    return std::nullopt;
  }

  return PrototypeInfo{
      .className = className,
      .funcName = kFuncName,
      .returnType = *kRetType,
      .reportNode = kMethodProto,
      .fileContent = fc,
  };
}

auto CollectPrototypes(const SL::FileContent* fc)
    -> std::vector<PrototypeInfo> {
  std::vector<PrototypeInfo> result;

  SL::NodeId const kRoot = fc->getRootNode();
  if (!kRoot) {
    return result;
  }

  for (SL::NodeId const kClassDecl :
       fc->sl_collect_all(kRoot, SL::VObjectType::paClass_declaration)) {
    std::string_view const kClassName = GetClassNameFromDecl(fc, kClassDecl);
    if (kClassName.empty()) {
      continue;
    }

    for (SL::NodeId const kClassItem :
         fc->sl_collect_all(kClassDecl, SL::VObjectType::paClass_item)) {
      std::optional<PrototypeInfo> const kInfo =
          TryCollectPrototype(fc, kClassItem, kClassName);
      if (kInfo.has_value()) {
        result.push_back(*kInfo);
      }
    }
  }

  return result;
}

struct ImplEntry {
  ReturnTypeInfo returnType;
};

auto CollectImplementations(const SL::FileContent* fc)
    -> std::unordered_map<std::string, ImplEntry> {
  std::unordered_map<std::string, ImplEntry> result;

  SL::NodeId const kRoot = fc->getRootNode();
  if (!kRoot) {
    return result;
  }

  for (SL::NodeId const kFuncDecl :
       fc->sl_collect_all(kRoot, SL::VObjectType::paFunction_declaration)) {
    SL::NodeId const kBodyDecl = fc->Child(kFuncDecl);
    if (!kBodyDecl ||
        fc->Type(kBodyDecl) != SL::VObjectType::paFunction_body_declaration) {
      continue;
    }

    auto const kMemberInfo =
        ClassScopeUtils::ExtractClassScopedMember(fc, kBodyDecl);
    if (!kMemberInfo.has_value()) {
      continue;
    }

    SL::NodeId const kFdtoi = fc->Child(kBodyDecl);
    if (!kFdtoi ||
        fc->Type(kFdtoi) != SL::VObjectType::paFunction_data_type_or_implicit) {
      continue;
    }

    std::optional<ReturnTypeInfo> const kRetType =
        ExtractReturnTypeInfo(fc, kFdtoi);
    if (!kRetType.has_value()) {
      continue;
    }

    std::string const kKey =
        MakeKey(kMemberInfo->className, kMemberInfo->memberName);

    result.insert_or_assign(kKey, ImplEntry{.returnType = *kRetType});
  }

  return result;
}

}  // namespace

void CheckFunctionImplementationReturnType(SL::Design* design,
                                           SL::ErrorContainer* errors,
                                           SL::SymbolTable* symbols) {
  if (design == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }

  std::vector<PrototypeInfo> allPrototypes;
  DesignUtils::ForEachFileContent(design, [&](const SL::FileContent* fc) {
    auto local = CollectPrototypes(fc);
    allPrototypes.insert(allPrototypes.end(), local.begin(), local.end());
  });

  if (allPrototypes.empty()) {
    return;
  }

  std::unordered_map<std::string, ImplEntry> allImpls;
  for (auto& [name, fc] : design->getAllFileContents()) {
    if (fc == nullptr) {
      continue;
    }
    auto localImpls = CollectImplementations(fc);
    allImpls.merge(localImpls);
  }

  for (const auto& proto : allPrototypes) {
    std::string const kKey = MakeKey(proto.className, proto.funcName);

    auto const kIt = allImpls.find(kKey);
    if (kIt == allImpls.end()) {
      continue;
    }

    if (ReturnTypesMatch(proto.returnType, kIt->second.returnType)) {
      continue;
    }

    ReportError(proto.fileContent, proto.reportNode, proto.funcName,
                verihogg_lint::LINT_FUNCTION_IMPLEMENTATION_RETURN_TYPE, errors,
                symbols);
  }
}