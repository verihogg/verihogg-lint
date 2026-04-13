#include "rules/method_implementation_argument_type.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/Design.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/ErrorReporting/ErrorDefinition.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <algorithm>
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

struct PrototypeInfo {
  std::string_view className;
  std::string_view funcName;
  std::vector<SvTypeInfo> argTypes;
  SL::NodeId reportNode;
  const SL::FileContent* fileContent;
};

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

  std::string_view const kFuncName =
      ClassScopeUtils::ExtractFuncNameFromPrototype(fc, kProto);
  if (kFuncName.empty()) {
    return std::nullopt;
  }

  SL::NodeId const kFdtoi = fc->Child(kProto);
  SL::NodeId const kPortList =
      FindSiblingOfType(fc, kFdtoi, SL::VObjectType::paTf_port_list);

  return PrototypeInfo{
      .className = className,
      .funcName = kFuncName,
      .argTypes = ExtractArgTypesFromPortList(fc, kPortList),
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
    std::string_view const kClassName =
        ClassScopeUtils::GetClassNameFromDecl(fc, kClassDecl);
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
  std::vector<SvTypeInfo> argTypes;
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

    SL::NodeId const kPortList =
        FindSiblingOfType(fc, kFdtoi, SL::VObjectType::paTf_port_list);

    std::string const kKey = ClassScopeUtils::MakeClassMethodKey(
        kMemberInfo->className, kMemberInfo->memberName);

    result.insert_or_assign(
        kKey,
        ImplEntry{.argTypes = ExtractArgTypesFromPortList(fc, kPortList)});
  }

  return result;
}

}  // namespace

void CheckMethodImplementationArgumentType(SL::Design* design,
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
    std::string const kKey =
        ClassScopeUtils::MakeClassMethodKey(proto.className, proto.funcName);

    auto const kIt = allImpls.find(kKey);
    if (kIt == allImpls.end()) {
      continue;
    }

    const auto& implArgTypes = kIt->second.argTypes;
    const std::size_t kCompareCount =
        std::min(proto.argTypes.size(), implArgTypes.size());

    for (std::size_t i = 0; i < kCompareCount; ++i) {
      if (!SvTypeInfosMatch(proto.argTypes.at(i), implArgTypes.at(i))) {
        ReportError(proto.fileContent, proto.reportNode, proto.funcName,
                    verihogg_lint::LINT_METHOD_IMPLEMENTATION_ARGUMENT_TYPE,
                    errors, symbols);
        break;
      }
    }
  }
}
