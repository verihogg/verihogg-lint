#pragma once

#include <string>
#include <vector>

#include "Surelog/Design/FileContent.h"
#include "Surelog/SourceCompile/VObjectTypes.h"

using namespace SURELOG;

static std::vector<NodeId> collectStringConsts(const FileContent* fC,
                                               NodeId node);

static std::string joinNames(const FileContent* fC,
                             const std::vector<NodeId>& parts);

void checkHierarchicalInterfaceIdentifier(const FileContent* fC,
                                          ErrorContainer* errors,
                                          SymbolTable* symbols);

