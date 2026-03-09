#include "rules/multiple_dot_star_connection.h"

#include <optional>
#include <string_view>

#include "Surelog/Design/FileContent.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"
#include "Surelog/SourceCompile/VObjectTypes.h"
#include "utils/location_utils.h"
#include "utils/name_utils.h"

using namespace SURELOG;

struct DotStarResult {
  NodeId secondDotStarNode;
  NodeId instanceNameNode;
};

static bool HasDotStarChild(const FileContent* fC, NodeId node) {
  return !fC->sl_collect_all(node, VObjectType::paDOTSTAR).empty();
}

static std::optional<DotStarResult> FindMultipleDotStarConnections(
    const FileContent* fC, NodeId instNode) {
  NodeId hierInst =
      fC->sl_collect(instNode, VObjectType::paHierarchical_instance);
  if (!hierInst) return std::nullopt;

  NodeId instanceNameNode = {};
  NodeId nameOfInst = fC->sl_collect(hierInst, VObjectType::paName_of_instance);
  if (nameOfInst) instanceNameNode = fC->Child(nameOfInst);

  NodeId portList =
      fC->sl_collect(hierInst, VObjectType::paList_of_port_connections);
  if (!portList) return std::nullopt;

  int dotStarCount = 0;
  for (NodeId child = fC->Child(portList); child; child = fC->Sibling(child)) {
    if (fC->Type(child) != VObjectType::paNamed_port_connection) continue;
    if (!HasDotStarChild(fC, child)) continue;

    if (++dotStarCount == 2) {
      NodeId dotStarNode = fC->sl_collect(child, VObjectType::paDOTSTAR);
      return DotStarResult{dotStarNode ? dotStarNode : child, instanceNameNode};
    }
  }

  return std::nullopt;
}

static void ReportMultipleDotStarError(const FileContent* fC, NodeId badNode,
                                       NodeId instanceNameNode,
                                       ErrorContainer* errors,
                                       SymbolTable* symbols) {
  if (!fC || !badNode || !errors || !symbols) return;

  std::string_view instanceName = "unknown";
  if (instanceNameNode) {
    instanceName = ExtractName(fC, instanceNameNode, "unknown");
  }

  ReportError(fC, badNode, instanceName,
              ErrorDefinition::LINT_MULTIPLE_DOT_STAR_CONNECTIONS, errors,
              symbols);
}

void CheckMultipleDotStarConnections(const FileContent* fC,
                                     ErrorContainer* errors,
                                     SymbolTable* symbols) {
  if (!fC || !errors || !symbols) return;

  NodeId root = fC->getRootNode();
  if (!root) return;

  for (NodeId inst :
       fC->sl_collect_all(root, VObjectType::paModule_instantiation)) {
    auto result = FindMultipleDotStarConnections(fC, inst);
    if (!result) continue;

    std::string_view instanceName =
        result->instanceNameNode
            ? ExtractName(fC, result->instanceNameNode, "unknown")
            : "unknown";

    ReportError(fC, result->secondDotStarNode, instanceName,
                ErrorDefinition::LINT_MULTIPLE_DOT_STAR_CONNECTIONS, errors,
                symbols);
  }
}
