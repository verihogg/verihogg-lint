#pragma once

#include <Surelog/Design/Design.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>

#include <filesystem>

#include "uhdm/vpi_user.h"

constexpr const char* DefaultConfigFileName = ".verihogg-lint";

void RunAllRules(SURELOG::ErrorContainer* errors,
                 SURELOG::SymbolTable* symbols);

void RunAllRulesOnDesign(SURELOG::Design* design, const vpiHandle& uhdmDesign,
                         SURELOG::ErrorContainer* errors,
                         SURELOG::SymbolTable* symbols,
                         std::filesystem::path configFile);

void DumpConfig();
