#pragma once

#include "Surelog/Design/FileContent.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"

using namespace SURELOG;

void checkConcatenationMultiplier(const FileContent* fC, ErrorContainer* errors,
                                  SymbolTable* symbols);
