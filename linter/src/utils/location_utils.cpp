#include "utils/location_utils.h"

#include <Surelog/Common/FileSystem.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>

namespace SL = SURELOG;

auto GetColumnSafe(const SL::FileContent* fileContent, SL::NodeId node) {
  if (fileContent == nullptr || !node) {
    return uint16_t{0};
  }
  try {
    return fileContent->Column(node);
  } catch (...) {
    return uint16_t{0};
  }
}

auto GetLocation(const SL::FileContent* fileContent, SL::NodeId node,
                 const std::string_view& symbolName, SL::SymbolTable* symbols) {
  if (fileContent == nullptr || !node || symbols == nullptr) {
    return SL::Location{SL::PathId{}, 0, 0,
                        symbols->registerSymbol(symbolName)};
  }

  return SL::Location{fileContent->getFileId(node), fileContent->Line(node),
                      static_cast<uint16_t>(GetColumnSafe(fileContent, node)),
                      symbols->registerSymbol(symbolName)};
}

void ReportError(const SL::FileContent* fileContent, SL::NodeId node,
                 const std::string_view& symbolName,
                 SL::ErrorDefinition::ErrorType errorType,
                 SL::ErrorContainer* errors, SL::SymbolTable* symbols) {
  if (fileContent == nullptr || !node || errors == nullptr ||
      symbols == nullptr) {
    return;
  }

  SL::Location loc = GetLocation(fileContent, node, symbolName, symbols);
  SL::Error err(errorType, loc);
  errors->addError(err, false);
}

auto FindArrayIdNode(const SL::FileContent* fileContent,
                     SL::NodeId foreachKeyword) -> SL::NodeId {
  for (SL::NodeId sib = fileContent->Sibling(foreachKeyword); sib;
       sib = fileContent->Sibling(sib)) {
    if (fileContent->Type(sib) ==
        SL::VObjectType::paPs_or_hierarchical_array_identifier) {
      return sib;
    }
  }
  return SL::InvalidNodeId;
}