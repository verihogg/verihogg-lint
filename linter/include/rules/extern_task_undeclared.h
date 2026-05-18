#pragma once

#include "Surelog/Design/Design.h"

void CheckExternTaskUndeclared(SURELOG::Design* design,
                               SURELOG::ErrorContainer* errors,
                               SURELOG::SymbolTable* symbols);
