#pragma once

#include "Surelog/Design/FileContent.h"

void CheckDuplicateClass(const SURELOG::FileContent* fileContent,
                         SURELOG::ErrorContainer* errors,
                         SURELOG::SymbolTable* symbols);
