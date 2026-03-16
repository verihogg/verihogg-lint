#pragma once

#include <Surelog/Design/Design.h>

void CheckDpiDeclarationString(const SURELOG::FileContent* fileContent,
                               SURELOG::ErrorContainer* errors,
                               SURELOG::SymbolTable* symbols);
