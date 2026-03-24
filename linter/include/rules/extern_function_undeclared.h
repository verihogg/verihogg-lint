#pragma once

#include "Surelog/Design/Design.h"

using namespace SURELOG;

void checkExternFunctionUndeclared(const FileContent* fC,
                                   ErrorContainer* errors,
                                   SymbolTable* symbols);
