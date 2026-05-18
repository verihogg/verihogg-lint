#include "rules/extend_interface_class.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/Design.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <cassert>
#include <cstddef>
#include <map>
#include <string>
#include <string_view>
#include <vector>

#include "main/lint_rules.h"
#include "utils/ast_utils.h"
#include "utils/design_utils.h"
#include "utils/location_utils.h"

namespace {
auto GetSuperclassStringsFromInterfaceClasses(
    const SURELOG::FileContent* fileContent, SURELOG::NodeId node)
    -> std::vector<std::string> {
  assert(fileContent->Type(node) != SURELOG::VObjectType::paClass_declaration);

  std::vector<std::string> result;
  const std::vector<SURELOG::NodeId> kClassType = fileContent->sl_collect_all(
      node, SURELOG::VObjectType::paInterface_class_type);
  for (const auto& typeId : kClassType) {
    const SURELOG::NodeId kIdent =
        fileContent->sl_get(typeId, SURELOG::VObjectType::paPs_identifier);
    const std::string superName =
        (!kIdent) ? "" : GetStringConst(fileContent, kIdent);
    result.push_back(superName);
  }

  return result;
}
}  // namespace

void CheckExtendInterfaceClass(SURELOG::Design* design,
                               SURELOG::ErrorContainer* errors,
                               SURELOG::SymbolTable* symbols) {
  if (design == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }
  std::map<std::string, std::vector<SURELOG::NodeId>> interfaceClassMap;
  DesignUtils::ForEachFileContent(
      design, [&](const SL::FileContent* fileContent) {
        const SURELOG::NodeId kRootNode = fileContent->getRootNode();
        const std::vector<SURELOG::NodeId> kInterfaceClassDeclarations =
            fileContent->sl_collect_all(
                kRootNode, SURELOG::VObjectType::paInterface_class_declaration);

        for (const auto& node : kInterfaceClassDeclarations) {
          const std::string kClassName = GetStringConst(fileContent, node);
          interfaceClassMap[kClassName].push_back(node);
        }
      });

  DesignUtils::ForEachFileContent(

      design, [&](const SL::FileContent* fileContent) {
        const SURELOG::NodeId kRootNode = fileContent->getRootNode();
        const std::vector<SURELOG::NodeId> kInterfaceClassDeclarations =
            fileContent->sl_collect_all(
                kRootNode, SURELOG::VObjectType::paInterface_class_declaration);
        for (const auto& interfaceId : kInterfaceClassDeclarations) {
          const std::string className =
              GetStringConst(fileContent, interfaceId);
          const std::string kMainPrefix = GetPrefix(fileContent, interfaceId);
          const std::vector<std::string> kSuperclasses =
              GetSuperclassStringsFromInterfaceClasses(fileContent,
                                                       interfaceId);

          for (const auto& superclassName : kSuperclasses) {
            if (IsBuiltinClass(className) || superclassName == "") {
              continue;
            }

            const SURELOG::NodeId kExtendsId = fileContent->sl_get(
                interfaceId, SURELOG::VObjectType::paEXTENDS);
            if (!kExtendsId) {
              continue;
            }

            const std::vector<SURELOG::NodeId> kSuperIdVector =
                interfaceClassMap[superclassName];
            bool found = false;
            for (const auto& superId : kSuperIdVector) {
              const std::string kSuperPrefix = GetPrefix(fileContent, superId);

              const size_t kSuperSize = kSuperPrefix.size();
              const size_t kMainSize = kSuperPrefix.size();
              if (kSuperSize <= kMainSize &&
                  std::string_view(kMainPrefix).substr(0, kSuperSize) ==
                      kSuperPrefix) {
                found = true;
                break;
              }
            }

            if (found) {
              continue;
            }

            ReportError(fileContent, interfaceId, className,
                        verihogg_lint::LINT_EXTEND_INTERFACE_CLASS, errors,
                        symbols);
          }
        }
      });
}
