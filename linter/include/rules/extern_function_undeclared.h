#pragma once

#include "Surelog/Design/Design.h"

void CheckExternFunctionUndeclared(SURELOG::Design* design,
                                   SURELOG::ErrorContainer* errors,
                                   SURELOG::SymbolTable* symbols);
