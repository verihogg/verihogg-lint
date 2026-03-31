#pragma once
#include <Surelog/Design/Design.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>

void CheckDuplicateEnumLiteral(SURELOG::Design* design,
                               SURELOG::ErrorContainer* errors,
                               SURELOG::SymbolTable* symbols);
