#pragma once

#include <string>

#include "Surelog/Design/Design.h"
#include "Surelog/Design/FileContent.h"
#include "Surelog/SourceCompile/VObjectTypes.h"

using namespace SURELOG;

void checkParameterDynamicArray(const FileContent* fC, ErrorContainer* errors,
                                SymbolTable* symbols);

std::string findParamName(const FileContent* fC, NodeId paramDeclId);

