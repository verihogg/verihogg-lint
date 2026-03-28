#pragma once

#include "Surelog/Design/Design.h"

void CheckCircularInheritance(const SURELOG::FileContent* fileContent,
                              SURELOG::ErrorContainer* errors,
                              SURELOG::SymbolTable* symbols);
