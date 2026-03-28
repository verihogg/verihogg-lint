#pragma once

#include "Surelog/Design/Design.h"

void CheckExternConstraintUndeclared(const SURELOG::FileContent* fileContent,
                                     SURELOG::ErrorContainer* errors,
                                     SURELOG::SymbolTable* symbols);
