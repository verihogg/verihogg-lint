#pragma once

#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>

void CheckSelectInEventControl(const SURELOG::FileContent* fileContent,
                               SURELOG::ErrorContainer* errors,
                               SURELOG::SymbolTable* symbols);
