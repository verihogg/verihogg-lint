#pragma once

#include "Surelog/Design/Design.h"

void checkClassVariableLifetime(const SURELOG::FileContent* fC,
                                SURELOG::ErrorContainer* errors,
                                SURELOG::SymbolTable* symbols);
