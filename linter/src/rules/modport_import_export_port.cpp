#include "rules/modport_import_export_port.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <string>
#include <unordered_set>
#include <utility>

#include "main/lint_rules.h"
#include "utils/location_utils.h"

namespace SL = SURELOG;

namespace {

auto GetTaskName(const SL::FileContent* fc, SL::NodeId taskDecl)
    -> std::string {
  for (SL::NodeId const kBody :
       fc->sl_collect_all(taskDecl, SL::VObjectType::paTask_body_declaration)) {
    SL::NodeId const kName = fc->Child(kBody);
    if (kName && fc->Type(kName) == SL::VObjectType::slStringConst) {
      return std::string(fc->SymName(kName));
    }
  }
  return {};
}

auto GetFunctionName(const SL::FileContent* fc, SL::NodeId funcDecl)
    -> std::string {
  for (SL::NodeId const kBody : fc->sl_collect_all(
           funcDecl, SL::VObjectType::paFunction_body_declaration)) {
    for (SL::NodeId ch = fc->Child(kBody); ch; ch = fc->Sibling(ch)) {
      if (fc->Type(ch) == SL::VObjectType::slStringConst) {
        return std::string(fc->SymName(ch));
      }
    }
  }
  return {};
}

auto CollectInterfaceMethodNames(const SL::FileContent* fc,
                                 SL::NodeId interfaceDecl)
    -> std::unordered_set<std::string> {
  std::unordered_set<std::string> methods;

  for (SL::NodeId const kTask :
       fc->sl_collect_all(interfaceDecl, SL::VObjectType::paTask_declaration)) {
    std::string name = GetTaskName(fc, kTask);
    if (!name.empty()) {
      methods.insert(std::move(name));
    }
  }

  for (SL::NodeId const kFunc : fc->sl_collect_all(
           interfaceDecl, SL::VObjectType::paFunction_declaration)) {
    std::string name = GetFunctionName(fc, kFunc);
    if (!name.empty()) {
      methods.insert(std::move(name));
    }
  }

  return methods;
}

void CheckModportsInInterface(const SL::FileContent* fc,
                              SL::NodeId interfaceDecl,
                              const std::unordered_set<std::string>& methods,
                              SL::ErrorContainer* errors,
                              SL::SymbolTable* symbols) {
  for (SL::NodeId const kTfDecl : fc->sl_collect_all(
           interfaceDecl, SL::VObjectType::paModport_tf_ports_declaration)) {
    for (SL::NodeId const kTfPort :
         fc->sl_collect_all(kTfDecl, SL::VObjectType::paModport_tf_port)) {
      SL::NodeId const kNameNode = fc->Child(kTfPort);
      if (!kNameNode || fc->Type(kNameNode) != SL::VObjectType::slStringConst) {
        continue;
      }

      std::string const name(fc->SymName(kNameNode));

      if (methods.find(name) == methods.end()) {
        ReportError(fc, kNameNode, name,
                    verihogg_lint::LINT_MODPORT_IMPORT_EXPORT_PORT, errors,
                    symbols);
      }
    }
  }
}

}  // namespace

void CheckModportImportExportPort(const SL::FileContent* fileContent,
                                  SL::ErrorContainer* errors,
                                  SL::SymbolTable* symbols) {
  if (fileContent == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }

  SL::NodeId const kRoot = fileContent->getRootNode();
  if (kRoot == SL::InvalidNodeId) {
    return;
  }

  for (SL::NodeId const kInterface : fileContent->sl_collect_all(
           kRoot, SL::VObjectType::paInterface_declaration)) {
    auto const methods = CollectInterfaceMethodNames(fileContent, kInterface);
    CheckModportsInInterface(fileContent, kInterface, methods, errors, symbols);
  }
}