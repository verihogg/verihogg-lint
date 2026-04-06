#include "rules/duplicate_coverpoint.h"

#include <Surelog/Common/FileSystem.h>
#include <Surelog/Common/NodeId.h>
#include <Surelog/Common/PathId.h>
#include <Surelog/Design/Design.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <map>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include "main/lint_rules.h"
#include "utils/location_utils.h"

namespace {

auto getCoverpointLabel(const SURELOG::FileContent* fc, SURELOG::NodeId cpNode)
    -> std::string_view {
  SURELOG::NodeId child = fc->Child(cpNode);
  while (child != SURELOG::InvalidNodeId) {
    const SURELOG::VObjectType type = fc->Type(child);
    if (type == SURELOG::VObjectType::slStringConst ||
        type == SURELOG::VObjectType::paIdentifier ||
        type == SURELOG::VObjectType::paPs_identifier ||
        type == SURELOG::VObjectType::paSimple_identifier ||
        type == SURELOG::VObjectType::paEscaped_identifier) {
      return fc->SymName(child);
    }
    child = fc->Sibling(child);
  }
  return "";
}

auto findCovergroupParent(const SURELOG::FileContent* fc, SURELOG::NodeId node)
    -> SURELOG::NodeId {
  while (node != SURELOG::InvalidNodeId) {
    const SURELOG::VObjectType type = fc->Type(node);
    if (type == SURELOG::VObjectType::paCovergroup_declaration ||
        type == SURELOG::VObjectType::paCOVERGROUP) {
      return node;
    }
    node = fc->Parent(node);
  }
  return SURELOG::InvalidNodeId;
}

}  

void CheckDuplicateCoverpoint(SURELOG::Design* design,
                              SURELOG::ErrorContainer* errors,
                              SURELOG::SymbolTable* symbols) {
  if (!design || !errors || !symbols) {
    return;
  }

  struct Location {
    SURELOG::PathId fileId{};
    unsigned line{};
  };

  const SURELOG::VObjectTypeUnorderedSet cpTypes = {
      SURELOG::VObjectType::paCover_point, SURELOG::VObjectType::paCOVERPOINT};

  using Key = std::pair<const SURELOG::FileContent*, SURELOG::NodeId>;
  std::map<Key, std::unordered_map<std::string, Location>> covergroupLabels;

  for (const auto& [fileId, fileContent] : design->getAllFileContents()) {
    if (!fileContent) {
      continue;
    }

    const std::vector<SURELOG::NodeId> cpNodes =
        fileContent->sl_collect_all(fileContent->getRootNode(), cpTypes, false);

    for (const SURELOG::NodeId cpNode : cpNodes) {
      const std::string_view label = getCoverpointLabel(fileContent, cpNode);
      if (label.empty()) {
        continue;
      }

      const SURELOG::NodeId covergroupNode =
          findCovergroupParent(fileContent, cpNode);
      if (covergroupNode == SURELOG::InvalidNodeId) {
        continue;
      }

      const Key key = std::make_pair(fileContent, covergroupNode);
      const Location loc{.fileId = fileContent->getFileId(cpNode),
                         .line = fileContent->Line(cpNode)};

      auto& labelMap = covergroupLabels[key];
      const auto it = labelMap.find(std::string(label));
      if (it != labelMap.end()) {
        const Location& first = it->second;
        const std::string_view firstPath =
            SURELOG::FileSystem::getInstance()->toPath(first.fileId);
        const std::string_view secondPath =
            SURELOG::FileSystem::getInstance()->toPath(loc.fileId);
        std::ostringstream context;
        context << "'" << label << "' (first at " << firstPath << ":"
                << first.line << ", duplicate at " << secondPath << ":"
                << loc.line << ")";

        ReportError(fileContent, cpNode, context.str(),
                    verihogg_lint::LINT_DUPLICATE_COVERPOINT, errors, symbols);
      } else {
        labelMap[std::string(label)] = loc;
      }
    }
  }
}
