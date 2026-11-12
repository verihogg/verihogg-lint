#include "rules/banned_psprintf.h"

#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>

#include <array>
#include <string_view>

#include "main/lint_rules.h"
#include "rules/banned_system_call_common.h"

namespace SL = SURELOG;

static constexpr std::array<std::string_view, 1> kPsprintf = {"psprintf"};

void CheckBannedPsprintf(const SL::FileContent* fileContent,
                         SL::ErrorContainer* errors, SL::SymbolTable* symbols) {
  CheckBannedSystemCallsImpl(fileContent, errors, symbols, kPsprintf,
                             verihogg_lint::LINT_BANNED_PSPRINTF);
}
