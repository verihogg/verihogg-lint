#pragma once

#include "Surelog/Design/FileContent.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"

void CheckAssignmentPattern(const SURELOG::FileContent* fC,
                            SURELOG::ErrorContainer* errors,
                            SURELOG::SymbolTable* symbols);
