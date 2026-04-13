#include "rules/method_override_argument_name.h"

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
#include "utils/design_utils.h"
#include "utils/location_utils.h"

namespace SL = SURELOG;

namespace {

struct ArgInfo {
  std::string name;
  SL::NodeId reportNode;
  const SL::FileContent* fileContent;
};

struct VirtualMethodInfo {
  std::string methodName;
  std::vector<ArgInfo> args;
};

struct ClassInfo {
  std::string className;
  std::string parentClassName;
  std::vector<VirtualMethodInfo> virtualMethods;
};

auto GetClassNameNode(const SL::FileContent* fc, SL::NodeId classDecl)
    -> SL::NodeId {
  SL::NodeId const kClassKw = fc->Child(classDecl);
  if (!kClassKw || fc->Type(kClassKw) != SL::VObjectType::paCLASS) {
    return SL::InvalidNodeId;
  }
  SL::NodeId const kNameNode = fc->Sibling(kClassKw);
  if (!kNameNode || fc->Type(kNameNode) != SL::VObjectType::slStringConst) {
    return SL::InvalidNodeId;
  }
  return kNameNode;
}

auto GetParentClassName(const SL::FileContent* fc, SL::NodeId classDecl)
    -> std::string_view {
  SL::NodeId const kClassKw = fc->Child(classDecl);
  if (!kClassKw) {
    return {};
  }

  SL::NodeId const kExtendsKw =
      FindSiblingOfType(fc, kClassKw, SL::VObjectType::paEXTENDS);
  if (!kExtendsKw) {
    return {};
  }

  SL::NodeId const kClassType =
      FindSiblingOfType(fc, kExtendsKw, SL::VObjectType::paClass_type);
  if (!kClassType) {
    return {};
  }

  SL::NodeId const kParentName = fc->Child(kClassType);
  if (!kParentName || fc->Type(kParentName) != SL::VObjectType::slStringConst) {
    return {};
  }
  return fc->SymName(kParentName);
}

auto ExtractArgs(const SL::FileContent* fc, SL::NodeId portList)
    -> std::vector<ArgInfo> {
  std::vector<ArgInfo> args;
  if (!portList) {
    return args;
  }

  for (SL::NodeId item = fc->Child(portList); item; item = fc->Sibling(item)) {
    if (fc->Type(item) != SL::VObjectType::paTf_port_item) {
      continue;
    }

    SL::NodeId const kTypeNode = fc->Child(item);
    if (!kTypeNode) {
      continue;
    }

    SL::NodeId const kArgNameNode = fc->Sibling(kTypeNode);
    if (!kArgNameNode ||
        fc->Type(kArgNameNode) != SL::VObjectType::slStringConst) {
      continue;
    }

    args.push_back(ArgInfo{
        .name = std::string{fc->SymName(kArgNameNode)},
        .reportNode = kArgNameNode,
        .fileContent = fc,
    });
  }
  return args;
}

auto ExtractFunctionSignature(const SL::FileContent* fc, SL::NodeId bodyDecl)
    -> std::optional<VirtualMethodInfo> {
  SL::NodeId const kRetType = fc->Child(bodyDecl);
  if (!kRetType ||
      fc->Type(kRetType) != SL::VObjectType::paFunction_data_type_or_implicit) {
    return std::nullopt;
  }

  SL::NodeId const kNameNode = fc->Sibling(kRetType);
  if (!kNameNode || fc->Type(kNameNode) != SL::VObjectType::slStringConst) {
    return std::nullopt;
  }

  SL::NodeId const kPortList =
      FindSiblingOfType(fc, kNameNode, SL::VObjectType::paTf_port_list);

  return VirtualMethodInfo{
      .methodName = std::string{fc->SymName(kNameNode)},
      .args = ExtractArgs(fc, kPortList),
  };
}

auto ExtractTaskSignature(const SL::FileContent* fc, SL::NodeId bodyDecl)
    -> std::optional<VirtualMethodInfo> {
  SL::NodeId const kNameNode = fc->Child(bodyDecl);
  if (!kNameNode || fc->Type(kNameNode) != SL::VObjectType::slStringConst) {
    return std::nullopt;
  }

  SL::NodeId const kPortList =
      FindSiblingOfType(fc, kNameNode, SL::VObjectType::paTf_port_list);

  return VirtualMethodInfo{
      .methodName = std::string{fc->SymName(kNameNode)},
      .args = ExtractArgs(fc, kPortList),
  };
}

auto TryExtractVirtualMethod(const SL::FileContent* fc, SL::NodeId classMethod)
    -> std::optional<VirtualMethodInfo> {
  SL::NodeId const kFirst = fc->Child(classMethod);
  if (!kFirst ||
      fc->Type(kFirst) != SL::VObjectType::paMethodQualifier_Virtual) {
    return std::nullopt;
  }

  SL::NodeId const kFuncDecl =
      FindChildOfType(fc, classMethod, SL::VObjectType::paFunction_declaration);
  if (kFuncDecl) {
    SL::NodeId const kBodyDecl = fc->Child(kFuncDecl);
    if (kBodyDecl &&
        fc->Type(kBodyDecl) == SL::VObjectType::paFunction_body_declaration) {
      return ExtractFunctionSignature(fc, kBodyDecl);
    }
    return std::nullopt;
  }

  SL::NodeId const kTaskDecl =
      FindChildOfType(fc, classMethod, SL::VObjectType::paTask_declaration);
  if (kTaskDecl) {
    SL::NodeId const kBodyDecl = fc->Child(kTaskDecl);
    if (kBodyDecl &&
        fc->Type(kBodyDecl) == SL::VObjectType::paTask_body_declaration) {
      return ExtractTaskSignature(fc, kBodyDecl);
    }
  }

  return std::nullopt;
}

auto CollectVirtualMethods(const SL::FileContent* fc, SL::NodeId classDecl)
    -> std::vector<VirtualMethodInfo> {
  std::vector<VirtualMethodInfo> result;

  for (SL::NodeId const kClassItem :
       fc->sl_collect_all(classDecl, SL::VObjectType::paClass_item)) {
    SL::NodeId const kClassMethod = fc->Child(kClassItem);
    if (!kClassMethod ||
        fc->Type(kClassMethod) != SL::VObjectType::paClass_method) {
      continue;
    }

    std::optional<VirtualMethodInfo> info =
        TryExtractVirtualMethod(fc, kClassMethod);
    if (info.has_value()) {
      result.push_back(std::move(*info));
    }
  }
  return result;
}

auto CollectClassInfosFromFile(const SL::FileContent* fc)
    -> std::vector<ClassInfo> {
  std::vector<ClassInfo> result;

  SL::NodeId const kRoot = fc->getRootNode();
  if (!kRoot) {
    return result;
  }

  for (SL::NodeId const kClassDecl :
       fc->sl_collect_all(kRoot, SL::VObjectType::paClass_declaration)) {
    SL::NodeId const kNameNode = GetClassNameNode(fc, kClassDecl);
    if (!kNameNode) {
      continue;
    }

    result.push_back(ClassInfo{
        .className = std::string{fc->SymName(kNameNode)},
        .parentClassName = std::string{GetParentClassName(fc, kClassDecl)},
        .virtualMethods = CollectVirtualMethods(fc, kClassDecl),
    });
  }
  return result;
}

struct GlobalClassEntry {
  std::string parentClassName;
  std::unordered_map<std::string, std::vector<std::string>> methods;
};

using GlobalMap = std::unordered_map<std::string, GlobalClassEntry>;

auto BuildGlobalMap(const std::vector<ClassInfo>& allClasses) -> GlobalMap {
  GlobalMap result;
  for (const auto& cls : allClasses) {
    auto& entry = result[cls.className];
    entry.parentClassName = cls.parentClassName;

    for (const auto& method : cls.virtualMethods) {
      std::vector<std::string> names;
      names.reserve(method.args.size());
      for (const auto& arg : method.args) {
        names.push_back(arg.name);
      }
      entry.methods.try_emplace(method.methodName, std::move(names));
    }
  }
  return result;
}

auto FindBaseArgNames(const std::string& parentClassName,
                      const GlobalMap& globalMap, const std::string& methodName)
    -> const std::vector<std::string>* {
  const std::string* current = &parentClassName;
  while (!current->empty()) {
    auto const kClassIt = globalMap.find(*current);
    if (kClassIt == globalMap.end()) {
      return nullptr;
    }

    auto const kMethodIt = kClassIt->second.methods.find(methodName);
    if (kMethodIt != kClassIt->second.methods.end()) {
      return &kMethodIt->second;
    }

    current = &kClassIt->second.parentClassName;
  }
  return nullptr;
}

auto CheckClass(const ClassInfo& childClass, const GlobalMap& globalMap,
                SL::ErrorContainer* errors, SL::SymbolTable* symbols) -> void {
  if (childClass.parentClassName.empty()) {
    return;
  }

  for (const auto& childMethod : childClass.virtualMethods) {
    const std::vector<std::string>* baseArgs = FindBaseArgNames(
        childClass.parentClassName, globalMap, childMethod.methodName);
    if (baseArgs == nullptr) {
      continue;
    }

    const std::size_t kCompareCount =
        std::min(baseArgs->size(), childMethod.args.size());

    for (std::size_t i = 0; i < kCompareCount; ++i) {
      if ((*baseArgs).at(i) == childMethod.args.at(i).name) {
        continue;
      }

      ReportError(
          childMethod.args.at(i).fileContent, childMethod.args.at(i).reportNode,
          childMethod.args.at(i).name,
          verihogg_lint::LINT_METHOD_OVERRIDE_ARGUMENT_NAME, errors, symbols);
    }
  }
}

}  // namespace

void CheckMethodOverrideArgumentName(SL::Design* design,
                                     SL::ErrorContainer* errors,
                                     SL::SymbolTable* symbols) {
  if (design == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }

  std::vector<ClassInfo> allClasses;
  DesignUtils::ForEachFileContent(design, [&](const SL::FileContent* fc) {
    auto local = CollectClassInfosFromFile(fc);
    allClasses.insert(allClasses.end(), local.begin(), local.end());
  });

  if (allClasses.empty()) {
    return;
  }

  GlobalMap const kGlobalMap = BuildGlobalMap(allClasses);

  for (const auto& classInfo : allClasses) {
    CheckClass(classInfo, kGlobalMap, errors, symbols);
  }
}
