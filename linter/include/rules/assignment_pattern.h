#pragma once

#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>

#include <vector>

#include "fix/source_manager.h"
#include "main/lint_diagnostics.h"

auto CheckAssignmentPatternFixable(const SURELOG::FileContent* fileContent,
                                   SURELOG::ErrorContainer* errors,
                                   SURELOG::SymbolTable* symbols,
                                   FixSourceManager& sm)
    -> std::vector<LintDiagnostic>;
