#pragma once

#include "Surelog/Design/Design.h"

using namespace SURELOG;

std::string findVariableName(const FileContent* fC, NodeId propId);

void checkClassVariableLifetime(const FileContent* fC, ErrorContainer* errors, SymbolTable* symbols);
