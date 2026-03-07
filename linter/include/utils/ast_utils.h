#pragma once

#include <cstdint>
#include <string_view>

#include "Surelog/Design/FileContent.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"
#include "Surelog/SourceCompile/VObjectTypes.h"

using namespace SURELOG;

NodeId findEnclosingModule(const FileContent* fC, NodeId node);

bool hasSiblingOfType(const FileContent* fC, NodeId start, VObjectType type);