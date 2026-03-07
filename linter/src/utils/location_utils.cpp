#include "utils/location_utils.h"

#include "Surelog/Common/FileSystem.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"

using namespace SURELOG;

uint32_t getColumnSafe(const FileContent* fC, NodeId node) {
  if (!fC || !node) return 0;
  try {
    return fC->Column(node);
  } catch (...) {
    return 0;
  }
}

Location getLocation(const FileContent* fC, NodeId node,
                     const std::string_view& symbolName, SymbolTable* symbols) {
  if (!fC || !node || !symbols) {
    PathId fileId;
    return Location(fileId, 0, 0, symbols->registerSymbol(symbolName));
  }

  PathId fileId = fC->getFileId(node);
  uint32_t line = fC->Line(node);
  uint32_t column = getColumnSafe(fC, node);
  SymbolId obj = symbols->registerSymbol(symbolName);

  return Location(fileId, line, column, obj);
}

void reportError(const FileContent* fC, NodeId node,
                 const std::string_view& symbolName,
                 ErrorDefinition::ErrorType errorType, ErrorContainer* errors,
                 SymbolTable* symbols) {
  if (!fC || !node || !errors || !symbols) return;

  Location loc = getLocation(fC, node, symbolName, symbols);
  Error err(errorType, loc);
  errors->addError(err, false);
}

NodeId findArrayIdNode(const FileContent* fC, NodeId foreachKeyword) {
  for (NodeId sib = fC->Sibling(foreachKeyword); sib; sib = fC->Sibling(sib)) {
    if (fC->Type(sib) == VObjectType::paPs_or_hierarchical_array_identifier)
      return sib;
  }
  return InvalidNodeId;
}