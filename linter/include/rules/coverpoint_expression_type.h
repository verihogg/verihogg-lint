#pragma once

#include <string>

#include "Surelog/Design/FileContent.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"
#include "Surelog/SourceCompile/VObjectTypes.h"

using namespace SURELOG;

void checkCoverpointExpressionType(const FileContent* fC,
                                   ErrorContainer* errors,
                                   SymbolTable* symbols);

bool isIntegralType(VObjectType type);

std::string getCoverpointName(const FileContent* fC, NodeId cpNode);

void checkSingleCoverpoint(const FileContent* fC, NodeId cpId,
                           ErrorContainer* errors, SymbolTable* symbols);

