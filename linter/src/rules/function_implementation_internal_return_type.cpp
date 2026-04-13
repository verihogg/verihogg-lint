#include "rules/function_implementation_internal_return_type.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/Design.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

#include "main/lint_rules.h"
#include "utils/ast_utils.h"
#include "utils/class_scope_utils.h"
#include "utils/design_utils.h"
#include "utils/location_utils.h"

namespace SL = SURELOG;

namespace {

using ClassTypedefsMap =
    std::unordered_map<std::string, std::unordered_set<std::string>>;

auto BuildClassTypedefsMap(SL::Design* design) -> ClassTypedefsMap {
  ClassTypedefsMap result;

  DesignUtils::ForEachFileContent(design, [&](const SL::FileContent* fc) {
    SL::NodeId const kRoot = fc->getRootNode();
    if (!kRoot) {
      return;
    }

    for (SL::NodeId const kClassDecl :
         fc->sl_collect_all(kRoot, SL::VObjectType::paClass_declaration)) {
      SL::NodeId const kClassKw = fc->Child(kClassDecl);
      if (!kClassKw || fc->Type(kClassKw) != SL::VObjectType::paCLASS) {
        continue;
      }
      SL::NodeId const kClassNameNode = fc->Sibling(kClassKw);
      if (!kClassNameNode ||
          fc->Type(kClassNameNode) != SL::VObjectType::slStringConst) {
        continue;
      }

      std::string className(fc->SymName(kClassNameNode));
      auto& entry = result[std::move(className)];
      for (std::string_view const sv :
           CollectUserDefinedTypes(fc, kClassDecl)) {
        entry.emplace(sv);
      }
    }
  });

  return result;
}

auto GetUnqualifiedReturnTypeNode(const SL::FileContent* fc,
                                  SL::NodeId funcBodyDecl) -> SL::NodeId {
  SL::NodeId const kFdtoi = fc->Child(funcBodyDecl);
  if (!kFdtoi ||
      fc->Type(kFdtoi) != SL::VObjectType::paFunction_data_type_or_implicit) {
    return SL::InvalidNodeId;
  }

  SL::NodeId const kFuncDataType = fc->Child(kFdtoi);
  if (!kFuncDataType ||
      fc->Type(kFuncDataType) != SL::VObjectType::paFunction_data_type) {
    return SL::InvalidNodeId;
  }

  SL::NodeId const kDataType = fc->Child(kFuncDataType);
  if (!kDataType || fc->Type(kDataType) != SL::VObjectType::paData_type) {
    return SL::InvalidNodeId;
  }

  SL::NodeId const kTypeChild = fc->Child(kDataType);
  if (!kTypeChild) {
    return SL::InvalidNodeId;
  }

  if (fc->Type(kTypeChild) == SL::VObjectType::paClass_scope) {
    return SL::InvalidNodeId;
  }

  if (fc->Type(kTypeChild) == SL::VObjectType::slStringConst) {
    return kTypeChild;
  }

  return SL::InvalidNodeId;
}

}  // namespace

void CheckFunctionImplementationInternalReturnType(SL::Design* design,
                                                   SL::ErrorContainer* errors,
                                                   SL::SymbolTable* symbols) {
  if (design == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }

  ClassTypedefsMap const classTypedefs = BuildClassTypedefsMap(design);
  if (classTypedefs.empty()) {
    return;
  }

  DesignUtils::ForEachFileContent(design, [&](const SL::FileContent* fc) {
    SL::NodeId const kRoot = fc->getRootNode();
    if (!kRoot) {
      return;
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

      SL::NodeId const kRetTypeNode =
          GetUnqualifiedReturnTypeNode(fc, kBodyDecl);
      if (!kRetTypeNode) {
        continue;
      }

      std::string const kRetTypeName(fc->SymName(kRetTypeNode));

      auto const kClassIt =
          classTypedefs.find(std::string(kMemberInfo->className));
      if (kClassIt == classTypedefs.end()) {
        continue;
      }
      if (kClassIt->second.count(kRetTypeName) == 0) {
        continue;
      }

      ReportError(
          fc, kRetTypeNode, kMemberInfo->memberName,
          verihogg_lint::LINT_FUNCTION_IMPLEMENTATION_INTERNAL_RETURN_TYPE,
          errors, symbols);
    }
  });
}
