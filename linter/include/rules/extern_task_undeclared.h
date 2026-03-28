#pragma once

#include "Surelog/Design/Design.h"

void CheckExternTaskUndeclared(const SURELOG::FileContent* fileContent,
                               SURELOG::ErrorContainer* errors,
                               SURELOG::SymbolTable* symbols);
