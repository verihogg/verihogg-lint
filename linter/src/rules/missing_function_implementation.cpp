// rules/missing_function_implementation.cpp
#include "rules/missing_function_implementation.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/Design.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

#include "main/lint_rules.h"
#include "utils/ast_utils.h"
#include "utils/design_utils.h"
#include "utils/location_utils.h"

namespace SL = SURELOG;

namespace {

auto MakeKey(std::string_view className, std::string_view methodName)
    -> std::string {
  std::string key;
  key.reserve(className.size() + 2 + methodName.size());
  key.append(className);
  key.append("::");
  key.append(methodName);
  return key;
}

auto ExtractMethodNameFromPrototype(const SL::FileContent* fileContent,
                                    SL::NodeId methodProto)
    -> std::string_view {
  SL::NodeId const firstChild = fileContent->Child(methodProto);
  if (!firstChild) {
    return {};
  }

  SL::VObjectType const protoType = fileContent->Type(methodProto);

  if (protoType == SL::VObjectType::paFunction_prototype) {
    if (fileContent->Type(firstChild) !=
        SL::VObjectType::paFunction_data_type_or_implicit) {
      return {};
    }
    SL::NodeId const nameNode = fileContent->Sibling(firstChild);
    if (!nameNode ||
        fileContent->Type(nameNode) != SL::VObjectType::slStringConst) {
      return {};
    }
    return fileContent->SymName(nameNode);
  }

  if (protoType == SL::VObjectType::paTask_prototype) {
    if (fileContent->Type(firstChild) != SL::VObjectType::slStringConst) {
      return {};
    }
    return fileContent->SymName(firstChild);
  }

  return {};
}

struct ExternMethodInfo {
  std::string_view className;
  std::string_view methodName;
  SL::NodeId reportNode;
  const SL::FileContent* fileContent;
};

auto CollectExternMethods(const SL::FileContent* fileContent)
    -> std::vector<ExternMethodInfo> {
  std::vector<ExternMethodInfo> result;

  SL::NodeId const root = fileContent->getRootNode();
  if (!root) {
    return result;
  }

  for (SL::NodeId const classDecl : fileContent->sl_collect_all(
           root, SL::VObjectType::paClass_declaration)) {
    SL::NodeId const classKw = fileContent->Child(classDecl);
    if (!classKw || fileContent->Type(classKw) != SL::VObjectType::paCLASS) {
      continue;
    }
    SL::NodeId const classNameNode = fileContent->Sibling(classKw);
    if (!classNameNode ||
        fileContent->Type(classNameNode) != SL::VObjectType::slStringConst) {
      continue;
    }
    std::string_view const className = fileContent->SymName(classNameNode);

    for (SL::NodeId const classItem : fileContent->sl_collect_all(
             classDecl, SL::VObjectType::paClass_item)) {
      SL::NodeId const classMethod = fileContent->Child(classItem);
      if (!classMethod ||
          fileContent->Type(classMethod) != SL::VObjectType::paClass_method) {
        continue;
      }

      SL::NodeId const firstChild = fileContent->Child(classMethod);
      if (!firstChild || fileContent->Type(firstChild) !=
                             SL::VObjectType::paExtern_qualifier) {
        continue;
      }

      SL::NodeId const methodProtoContainer = FindSiblingOfType(
          fileContent, firstChild, SL::VObjectType::paMethod_prototype);
      if (!methodProtoContainer) {
        continue;
      }

      SL::NodeId const proto = fileContent->Child(methodProtoContainer);
      if (!proto) {
        continue;
      }

      SL::VObjectType const protoType = fileContent->Type(proto);
      if (protoType != SL::VObjectType::paFunction_prototype &&
          protoType != SL::VObjectType::paTask_prototype) {
        continue;
      }

      std::string_view const methodName =
          ExtractMethodNameFromPrototype(fileContent, proto);
      if (methodName.empty()) {
        continue;
      }

      result.push_back(ExternMethodInfo{
          .className = className,
          .methodName = methodName,
          .reportNode = methodProtoContainer,
          .fileContent = fileContent,
      });
    }
  }

  return result;
}

auto ExtractImplementationKey(const SL::FileContent* fileContent,
                              SL::NodeId bodyDecl) -> std::string {
  SL::NodeId const classScope =
      FindChildOfType(fileContent, bodyDecl, SL::VObjectType::paClass_scope);

  if (!classScope) {
    return {};
  }

  SL::NodeId const classType = fileContent->Child(classScope);
  if (!classType ||
      fileContent->Type(classType) != SL::VObjectType::paClass_type) {
    return {};
  }
  SL::NodeId const classNameNode = fileContent->Child(classType);
  if (!classNameNode ||
      fileContent->Type(classNameNode) != SL::VObjectType::slStringConst) {
    return {};
  }
  std::string_view const className = fileContent->SymName(classNameNode);

  SL::NodeId const methodNameNode = fileContent->Sibling(classScope);
  if (!methodNameNode ||
      fileContent->Type(methodNameNode) != SL::VObjectType::slStringConst) {
    return {};
  }
  std::string_view const methodName = fileContent->SymName(methodNameNode);

  return MakeKey(className, methodName);
}

auto CollectImplementedMethods(const SL::FileContent* fileContent)
    -> std::unordered_set<std::string> {
  std::unordered_set<std::string> result;

  SL::NodeId const root = fileContent->getRootNode();
  if (!root) {
    return result;
  }

  for (SL::NodeId const funcDecl : fileContent->sl_collect_all(
           root, SL::VObjectType::paFunction_declaration)) {
    SL::NodeId const bodyDecl = fileContent->Child(funcDecl);
    if (!bodyDecl || fileContent->Type(bodyDecl) !=
                         SL::VObjectType::paFunction_body_declaration) {
      continue;
    }
    std::string key = ExtractImplementationKey(fileContent, bodyDecl);
    if (!key.empty()) {
      result.insert(std::move(key));
    }
  }

  for (SL::NodeId const taskDecl :
       fileContent->sl_collect_all(root, SL::VObjectType::paTask_declaration)) {
    SL::NodeId const bodyDecl = fileContent->Child(taskDecl);
    if (!bodyDecl || fileContent->Type(bodyDecl) !=
                         SL::VObjectType::paTask_body_declaration) {
      continue;
    }
    std::string key = ExtractImplementationKey(fileContent, bodyDecl);
    if (!key.empty()) {
      result.insert(std::move(key));
    }
  }

  return result;
}

}  // namespace

void CheckMissingFunctionImplementation(SL::Design* design,
                                        SL::ErrorContainer* errors,
                                        SL::SymbolTable* symbols) {
  if (design == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }

  std::vector<ExternMethodInfo> allExternMethods;
  DesignUtils::ForEachFileContent(design, [&](const SL::FileContent* fc) {
    auto local = CollectExternMethods(fc);
    allExternMethods.insert(allExternMethods.end(), local.begin(), local.end());
  });

  if (allExternMethods.empty()) {
    return;
  }

  std::unordered_set<std::string> implementedMethods;
  for (auto& [name, fileContent] : design->getAllFileContents()) {
    if (fileContent == nullptr) {
      continue;
    }
    auto localImpl = CollectImplementedMethods(fileContent);
    implementedMethods.merge(localImpl);
  }

  for (const auto& externInfo : allExternMethods) {
    std::string const key =
        MakeKey(externInfo.className, externInfo.methodName);
    if (implementedMethods.count(key) > 0) {
      continue;
    }

    ReportError(
        externInfo.fileContent, externInfo.reportNode, externInfo.methodName,
        verihogg_lint::LINT_MISSING_FUNCTION_IMPLEMENTATION, errors, symbols);
  }
}