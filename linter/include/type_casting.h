#pragma once

#include <string>

#include "Surelog/Design/FileContent.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"

using namespace SURELOG;

namespace Analyzer {

void checkTypeCasting(const FileContent* fC, ErrorContainer* errors,
                      SymbolTable* symbols);

}  // namespace Analyzer