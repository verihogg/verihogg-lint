#pragma once

#include "Surelog/Design/Design.h"

void CheckDuplicateConstructor(const SURELOG::FileContent* fileContent,
                               SURELOG::ErrorContainer* errors,
                               SURELOG::SymbolTable* symbols);
