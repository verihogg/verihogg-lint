#include "rules/banned_simulation_control_tasks.h"

#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>

#include <array>
#include <string_view>

#include "main/lint_rules.h"
#include "rules/banned_system_call_common.h"

namespace SL = SURELOG;

static constexpr std::array<std::string_view, 3> kSimulationControlTasks = {
    "stop", "finish", "exit"};

void CheckBannedSimulationControlTasks(const SL::FileContent* fileContent,
                                       SL::ErrorContainer* errors,
                                       SL::SymbolTable* symbols) {
  CheckBannedSystemCallsImpl(
      fileContent, errors, symbols, kSimulationControlTasks,
      verihogg_lint::LINT_BANNED_SIMULATION_CONTROL_TASKS);
}
