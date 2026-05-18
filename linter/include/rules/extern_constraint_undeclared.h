#pragma once

#include "Surelog/Design/Design.h"

void CheckExternConstraintUndeclared(SURELOG::Design* design,
                                     SURELOG::ErrorContainer* errors,
                                     SURELOG::SymbolTable* symbols);
