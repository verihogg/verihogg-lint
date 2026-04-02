#include "utils/location_utils.h"

#include <Surelog/Common/FileSystem.h>
#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/Error.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/ErrorReporting/ErrorDefinition.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <cstdint>
#include <exception>
#include <string_view>

namespace SL = SURELOG;

auto GetColumnSafe(const SL::FileContent* fileContent, SL::NodeId node) {
  if (fileContent == nullptr || !node) {
    return uint16_t{0};
  }
  try {
    return fileContent->Column(node);
  } catch (const std::exception&) {
    return uint16_t{0};
  }
}

auto GetLocation(const SL::FileContent* fileContent, SL::NodeId node,
                 const std::string_view& symbolName, SL::SymbolTable* symbols) {
  if (symbols == nullptr) {
    return SL::Location{SL::PathId{}, 0, 0, SL::SymbolId{}};
  }

  if (fileContent == nullptr || !node) {
    return SL::Location{SL::PathId{}, 0, 0,
                        symbols->registerSymbol(symbolName)};
  }

  return SL::Location{fileContent->getFileId(node), fileContent->Line(node),
                      GetColumnSafe(fileContent, node),
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

  SL::Location const kLoc = GetLocation(fileContent, node, symbolName, symbols);
  SL::Error err(errorType, kLoc);
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