#include "rules/banned_program_blocks.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <string>

#include "main/lint_rules.h"
#include "utils/ast_utils.h"
#include "utils/location_utils.h"

namespace SL = SURELOG;

void CheckBannedProgramBlocks(const SL::FileContent* fileContent,
                              SL::ErrorContainer* errors,
                              SL::SymbolTable* symbols) {
  if (fileContent == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }

  SL::NodeId const kRoot = fileContent->getRootNode();
  if (!kRoot) {
    return;
  }

  for (SL::NodeId const kProgramId : fileContent->sl_collect_all(
           kRoot, SL::VObjectType::paProgram_declaration)) {
    std::string const kProgramName = GetStringConst(fileContent, kProgramId);
    ReportError(fileContent, kProgramId, kProgramName,
                verihogg_lint::LINT_BANNED_PROGRAM_BLOCKS, errors, symbols);
  }
}
