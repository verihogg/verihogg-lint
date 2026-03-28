#pragma once

#include "Surelog/Design/Design.h"

void CheckImplementInterfaceClass(const SURELOG::FileContent* fileContent,
                                  SURELOG::ErrorContainer* errors,
                                  SURELOG::SymbolTable* symbols);
