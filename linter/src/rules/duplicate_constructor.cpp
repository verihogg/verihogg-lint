#include "rules/duplicate_constructor.h"

#include <cassert>
#include <map>
#include <vector>

#include "Surelog/CommandLine/CommandLineParser.h"
#include "Surelog/Design/Design.h"
#include "Surelog/Design/FileContent.h"
#include "Surelog/Design/ModuleDefinition.h"
#include "Surelog/Design/ModuleInstance.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/Library/Library.h"
#include "Surelog/SourceCompile/CompileSourceFile.h"
#include "Surelog/SourceCompile/Compiler.h"
#include "Surelog/SourceCompile/ParseFile.h"
#include "Surelog/Testbench/ClassDefinition.h"
#include "utils/ast_utils.h"
#include "utils/location_utils.h"

using namespace SURELOG;

void checkDuplicateConstructor(const FileContent* fC, ErrorContainer* errors,
                               SymbolTable* symbols) {
  if (!fC) return;

  const std::vector<NodeId> classDeclarations =
      fC->sl_collect_all(fC->getRootNode(), VObjectType::paClass_declaration);

  for (auto& id : classDeclarations) {
    const std::string className = getStringConst(fC, id);
    std::map<std::vector<VObjectType>, bool> argTree;
    std::vector<NodeId> methods = fC->sl_get_all(id, VObjectType::paClass_item);

    for (auto& id : methods) {
      id = fC->sl_get(id, VObjectType::paClass_method);
      id = fC->sl_get(id, VObjectType::paClass_constructor_declaration);
      if (id == zeroId) continue;

      std::vector<VObjectType> types;
      std::vector<NodeId> arguments;

      id = fC->sl_get(id, VObjectType::paTf_port_list);
      std::vector<NodeId> items =
          fC->sl_get_all(id, VObjectType::paTf_port_item);
      for (auto& item : items) {
        item = fC->sl_get(item, VObjectType::paData_type_or_implicit);
        item = fC->sl_get(item, VObjectType::paData_type);
        item = fC->Child(item);
        if (item == zeroId) continue;

        const VObjectType type = fC->Type(item);
        types.push_back(type);
      }

      if (argTree.count(types) > 0) {
        reportError(fC, id, className,
                    ErrorDefinition::LINT_DUPLICATE_CONSTRUCTOR, errors,
                    symbols);
      }
      argTree[types] = true;
    }
  }
}

