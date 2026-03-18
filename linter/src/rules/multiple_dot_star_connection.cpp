#include "rules/multiple_dot_star_connection.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/ErrorReporting/ErrorDefinition.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <optional>
#include <string_view>

#include "utils/location_utils.h"
#include "utils/name_utils.h"

namespace SL = SURELOG;

struct DotStarResult {
  SL::NodeId secondDotStarNode;
  SL::NodeId instanceNameNode;
};

namespace {
auto HasDotStarChild(const SL::FileContent* fileContent, SL::NodeId node)
    -> bool {
  return !fileContent->sl_collect_all(node, SL::VObjectType::paDOTSTAR).empty();
}

auto FindMultipleDotStarConnections(const SL::FileContent* fileContent,
                                    SL::NodeId instNode)
    -> std::optional<DotStarResult> {
  SL::NodeId const hierInst = fileContent->sl_collect(
      instNode, SL::VObjectType::paHierarchical_instance);
  if (hierInst == SL::InvalidNodeId) {
    return std::nullopt;
  }

  SL::NodeId instanceNameNode = {};
  SL::NodeId const nameOfInst =
      fileContent->sl_collect(hierInst, SL::VObjectType::paName_of_instance);
  if (nameOfInst) {
    instanceNameNode = fileContent->Child(nameOfInst);
  }

  SL::NodeId const portList = fileContent->sl_collect(
      hierInst, SL::VObjectType::paList_of_port_connections);
  if (portList == SL::InvalidNodeId) {
    return std::nullopt;
  }

  int dotStarCount = 0;
  for (SL::NodeId child = fileContent->Child(portList); child;
       child = fileContent->Sibling(child)) {
    if (fileContent->Type(child) != SL::VObjectType::paNamed_port_connection) {
      continue;
    }
    if (!HasDotStarChild(fileContent, child)) {
      continue;
    }

    if (++dotStarCount == 2) {
      SL::NodeId const dotStarNode =
          fileContent->sl_collect(child, SL::VObjectType::paDOTSTAR);
      return DotStarResult{
          .secondDotStarNode = dotStarNode ? dotStarNode : child,
          .instanceNameNode = instanceNameNode};
    }
  }

  return std::nullopt;
}
}  // namespace

void CheckMultipleDotStarConnections(const SL::FileContent* fileContent,
                                     SL::ErrorContainer* errors,
                                     SL::SymbolTable* symbols) {
  if (fileContent == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }

  SL::NodeId const root = fileContent->getRootNode();
  if (root == SL::InvalidNodeId) {
    return;
  }

  for (SL::NodeId const inst : fileContent->sl_collect_all(
           root, SL::VObjectType::paModule_instantiation)) {
    auto result = FindMultipleDotStarConnections(fileContent, inst);
    if (!result) {
      continue;
    }

    std::string_view const instanceName =
        result->instanceNameNode
            ? ExtractName(fileContent, result->instanceNameNode, "unknown")
            : "unknown";

    ReportError(fileContent, result->secondDotStarNode, instanceName,
                SL::ErrorDefinition::LINT_MULTIPLE_DOT_STAR_CONNECTIONS, errors,
                symbols);
  }
}
