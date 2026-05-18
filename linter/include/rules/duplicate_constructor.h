#pragma once

#include "Surelog/Design/Design.h"

void CheckDuplicateConstructor(SURELOG::Design* design,
                               SURELOG::ErrorContainer* errors,
                               SURELOG::SymbolTable* symbols);
