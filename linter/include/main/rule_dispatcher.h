#pragma once

#include <Surelog/Design/Design.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>

void RunAllRules(SURELOG::ErrorContainer* errors,
                 SURELOG::SymbolTable* symbols);

void RunAllRulesOnDesign(SURELOG::Design* design, const vpiHandle& uhdmDesign,
                         SURELOG::ErrorContainer* errors,
                         SURELOG::SymbolTable* symbols);
