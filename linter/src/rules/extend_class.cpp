#include "rules/extend_class.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/Design.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <cstddef>
#include <map>
#include <string>
#include <string_view>
#include <vector>

#include "main/lint_rules.h"
#include "utils/ast_utils.h"
#include "utils/location_utils.h"

void CheckExtendClass(const SURELOG::FileContent* fileContent,
                      SURELOG::ErrorContainer* errors,
                      SURELOG::SymbolTable* symbols) {
  if (fileContent == nullptr) {
    return;
  }

  const std::vector<SURELOG::NodeId> kClassDeclarations =
      fileContent->sl_collect_all(fileContent->getRootNode(),
                                  SURELOG::VObjectType::paClass_declaration);

  std::map<std::string, std::vector<SURELOG::NodeId>> classMap;
  for (const auto& kId : kClassDeclarations) {
    const std::string kClassName = GetStringConst(fileContent, kId);
    classMap[kClassName].push_back(kId);
  }

  for (const auto& kId : kClassDeclarations) {
    const std::string kClassName = GetStringConst(fileContent, kId);
    const std::string kMainPrefix = GetPrefix(fileContent, kId);
    const std::string kSuperclassName = GetSuperclassString(fileContent, kId);

    const SURELOG::NodeId kExtendsId =
        fileContent->sl_get(kId, SURELOG::VObjectType::paEXTENDS);
    if (!kExtendsId) {
      continue;
    }

    if (IsBuiltinClass(kClassName) || kSuperclassName == "") {
      continue;
    }

    const std::vector<SURELOG::NodeId> kSuperIdVector =
        classMap[kSuperclassName];
    bool found = false;
    for (const auto& superId : kSuperIdVector) {
      const std::string kSuperPrefix = GetPrefix(fileContent, superId);

      const size_t kSuperSize = kSuperPrefix.size();
      const size_t kMainSize = kSuperPrefix.size();
      if (kSuperSize <= kMainSize &&
          std::string_view(kMainPrefix).substr(0, kSuperSize) == kSuperPrefix) {
        found = true;
        break;
      }
    }

    if (found) {
      continue;
    }

    ReportError(fileContent, kId, kSuperclassName,
                verihogg_lint::LINT_EXTEND_CLASS, errors, symbols);
  }
}
