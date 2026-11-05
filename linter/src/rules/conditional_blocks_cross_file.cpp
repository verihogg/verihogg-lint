#include "rules/conditional_blocks_cross_file.h"

#include <Surelog/Design/Design.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <vector>

#include "main/lint_rules.h"
#include "utils/location_utils.h"

namespace SL = SURELOG;

void CheckConditionalBlocksCrossFile(SL::Design* design,
                                     SL::ErrorContainer* errors,
                                     SL::SymbolTable* symbols) {
  if (design == nullptr) {
    return;
  }
  for (const auto& entry : design->getAllPPFileContents()) {
    SL::FileContent* ppFC = entry.second;
    if (ppFC == nullptr) {
      continue;
    }
    const std::vector<SL::NodeId> nodes = ppFC->sl_collect_all(
        ppFC->getRootNode(),
        SL::VObjectTypeUnorderedSet{SL::VObjectType::ppIfdef_directive,
                                    SL::VObjectType::ppIfndef_directive,
                                    SL::VObjectType::ppEndif_directive});

    std::vector<SL::NodeId> openBlocks;
    for (const SL::NodeId id : nodes) {
      if (ppFC->Type(id) == SL::VObjectType::ppEndif_directive) {
        if (openBlocks.empty()) {
          ReportError(ppFC, id, ppFC->getName(),
                      verihogg_lint::LINT_CONDITIONAL_BLOCKS_CROSS_FILE, errors,
                      symbols);
        } else {
          openBlocks.pop_back();
        }
      } else {
        openBlocks.push_back(id);
      }
    }
    for (const SL::NodeId id : openBlocks) {
      ReportError(ppFC, id, ppFC->getName(),
                  verihogg_lint::LINT_CONDITIONAL_BLOCKS_CROSS_FILE, errors,
                  symbols);
    }
  }
}