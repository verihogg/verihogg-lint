#include "rules/illegal_checker_instance.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <string>
#include <string_view>
#include <vector>

#include "main/lint_rules.h"
#include "utils/location_utils.h"
#include "utils/name_utils.h"

void CheckIllegalCheckerInstance(const SURELOG::FileContent* fileContent,
                                 SURELOG::ErrorContainer* errors,
                                 SURELOG::SymbolTable* symbols) {
  if (!fileContent || !errors || !symbols) {
    return;
  }

  SURELOG::NodeId root = fileContent->getRootNode();
  if (!root) {
    return;
  }

  std::vector<SURELOG::NodeId> allInst;

  auto modInst = fileContent->sl_collect_all(
      root, SURELOG::VObjectType::paModule_instantiation);
  auto chkInst = fileContent->sl_collect_all(
      root, SURELOG::VObjectType::paChecker_instantiation);

  allInst.insert(allInst.end(), modInst.begin(), modInst.end());
  allInst.insert(allInst.end(), chkInst.begin(), chkInst.end());

  for (SURELOG::NodeId instNode : allInst) {
    SURELOG::NodeId anc = instNode;
    while (anc != SURELOG::InvalidNodeId) {
      SURELOG::VObjectType t = fileContent->Type(anc);
      if (t == SURELOG::VObjectType::paChecker_declaration) {
        break;
      }
      if (t == SURELOG::VObjectType::paModule_declaration ||
          t == SURELOG::VObjectType::paInterface_declaration ||
          t == SURELOG::VObjectType::paProgram_declaration ||
          t == SURELOG::VObjectType::paPackage_declaration ||
          t == SURELOG::VObjectType::paClass_declaration) {
        anc = SURELOG::InvalidNodeId;
        break;
      }
      anc = fileContent->Parent(anc);
    }

    if (anc == SURELOG::InvalidNodeId) {
      continue;
    }
    std::string instName;
    auto strings = fileContent->sl_collect_all(
        instNode, SURELOG::VObjectType::slStringConst);
    if (!strings.empty()) {
      instName = std::string(fileContent->SymName(strings.front()));
    }
    if (instName.empty()) {
      for (SURELOG::NodeId child = fileContent->Child(instNode);
           child != SURELOG::InvalidNodeId;
           child = fileContent->Sibling(child)) {
        if (fileContent->Type(child) == SURELOG::VObjectType::paIdentifier) {
          SURELOG::SymbolId sym = fileContent->Name(child);
          instName = std::string(symbols->getSymbol(sym));
          break;
        }
      }
    }

    if (instName.empty()) {
      continue;
    }

    std::string_view checkerName = ExtractName(fileContent, anc);
    std::string msg = "Instance '" + instName +
                      "' of checker inside checker '" +
                      std::string(checkerName) + "'";

    ReportError(fileContent, instNode, msg,
                verihogg_lint::LINT_ILLEGAL_CHECKER_INSTANCE, errors, symbols);
  }
}
