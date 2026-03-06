#pragma once

#include "Surelog/Design/Design.h"
#include "Surelog/Design/FileContent.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"

void runAllRules(const SURELOG::FileContent* fC,
                 SURELOG::ErrorContainer* errors,
                 SURELOG::SymbolTable* symbols);

void runAllRulesOnDesign(SURELOG::Design* design, const vpiHandle& UHDMdesign,
                         SURELOG::ErrorContainer* errors,
                         SURELOG::SymbolTable* symbols);
