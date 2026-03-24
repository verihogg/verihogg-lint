#pragma once

#include "Surelog/Design/Design.h"

using namespace SURELOG;

void checkExternTaskUndeclared(const FileContent* fC, ErrorContainer* errors,
                               SymbolTable* symbols);
