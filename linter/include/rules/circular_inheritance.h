#pragma once

#include "Surelog/Design/Design.h"

using namespace SURELOG;

void checkCircularInheritance(const FileContent* fC, ErrorContainer* errors,
                              SymbolTable* symbols);
