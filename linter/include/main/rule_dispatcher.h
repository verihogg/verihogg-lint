#pragma once

#include <string>

#include "Surelog/Design/Design.h"
#include "Surelog/Design/FileContent.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"

using namespace SURELOG;

void runAllRules(const FileContent* fC, ErrorContainer* errors, SymbolTable* symbols);

void runAllRulesOnDesign(Design* design, const vpiHandle& UHDMdesign, ErrorContainer* errors, SymbolTable* symbols);
