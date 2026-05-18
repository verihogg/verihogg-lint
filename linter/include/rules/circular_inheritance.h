#pragma once

#include "Surelog/Design/Design.h"

void CheckCircularInheritance(SURELOG::Design* design,
                              SURELOG::ErrorContainer* errors,
                              SURELOG::SymbolTable* symbols);
