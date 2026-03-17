#pragma once

#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <cstdint>
#include <string_view>

using namespace SURELOG;

NodeId FindEnclosingModule(const FileContent* fC, NodeId node);

bool HasSiblingOfType(const FileContent* fC, NodeId start, VObjectType type);

NodeId FindAncestorOfType(const FileContent* fC, NodeId node, VObjectType type);