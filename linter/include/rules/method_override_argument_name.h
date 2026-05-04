#pragma once

#include <Surelog/Design/Design.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>

#include <vector>

#include "fix/source_manager.h"
#include "main/lint_diagnostics.h"

auto CheckMethodOverrideArgumentNameFixable(SURELOG::Design* design,
                                            SURELOG::ErrorContainer* errors,
                                            SURELOG::SymbolTable* symbols,
                                            FixSourceManager& sm)
    -> std::vector<LintDiagnostic>;
