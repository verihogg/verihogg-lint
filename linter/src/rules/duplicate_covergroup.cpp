#include "rules/duplicate_covergroup.h"

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

auto getCovergroupName(const SURELOG::FileContent* fc, SURELOG::NodeId node)
    -> std::string_view {
  SURELOG::NodeId child = fc->Child(node);
  while (child != SURELOG::InvalidNodeId) {
    const SURELOG::VObjectType type = fc->Type(child);
    if (type == SURELOG::VObjectType::slStringConst ||
        type == SURELOG::VObjectType::paIdentifier ||
        type == SURELOG::VObjectType::paPs_identifier ||
        type == SURELOG::VObjectType::paSimple_identifier) {
      return fc->SymName(child);
    }
    child = fc->Sibling(child);
  }
  return "";
}

auto findScopeParent(const SURELOG::FileContent* fc, SURELOG::NodeId node)
    -> SURELOG::NodeId {
  while (node != SURELOG::InvalidNodeId) {
    const SURELOG::VObjectType type = fc->Type(node);
    if (type == SURELOG::VObjectType::paModule_declaration ||
        type == SURELOG::VObjectType::paInterface_declaration ||
        type == SURELOG::VObjectType::paPackage_declaration ||
        type == SURELOG::VObjectType::paProgram_declaration ||
        type == SURELOG::VObjectType::paClass_declaration) {
      return node;
    }
    node = fc->Parent(node);
  }
  return SURELOG::InvalidNodeId;
}

}  // namespace

void CheckDuplicateCovergroup(SURELOG::Design* design,
                              SURELOG::ErrorContainer* errors,
                              SURELOG::SymbolTable* symbols) {
  if (!design || !errors || !symbols) {
    return;
  }

  struct Location {
    SURELOG::PathId fileId{};
    unsigned line{};
  };

  const SURELOG::VObjectTypeUnorderedSet covergroupTypes = {
      SURELOG::VObjectType::paCovergroup_declaration,
      SURELOG::VObjectType::paCOVERGROUP};

  using Key = std::pair<const SURELOG::FileContent*, SURELOG::NodeId>;
  std::map<Key, std::unordered_map<std::string, Location>> scopeMap;

  for (const auto& [fileId, fileContent] : design->getAllFileContents()) {
    if (!fileContent) {
      continue;
    }

    const std::vector<SURELOG::NodeId> cgNodes = fileContent->sl_collect_all(
        fileContent->getRootNode(), covergroupTypes, false);

    for (const SURELOG::NodeId cgNode : cgNodes) {
      const std::string_view name = getCovergroupName(fileContent, cgNode);
      if (name.empty()) {
        continue;
      }

      SURELOG::NodeId scopeNode = findScopeParent(fileContent, cgNode);
      if (scopeNode == SURELOG::InvalidNodeId) {
        scopeNode = fileContent->getRootNode();
      }

      const Key key = std::make_pair(fileContent, scopeNode);
      const Location loc{.fileId = fileContent->getFileId(cgNode),
                         .line = fileContent->Line(cgNode)};

      auto& nameMap = scopeMap[key];
      const auto it = nameMap.find(std::string(name));
      if (it != nameMap.end()) {
        const Location& first = it->second;
        const std::string_view firstPath =
            SURELOG::FileSystem::getInstance()->toPath(first.fileId);
        const std::string_view secondPath =
            SURELOG::FileSystem::getInstance()->toPath(loc.fileId);
        std::ostringstream context;
        context << "'" << name << "' (first at " << firstPath << ":"
                << first.line << ", duplicate at " << secondPath << ":"
                << loc.line << ")";

        ReportError(fileContent, cgNode, context.str(),
                    verihogg_lint::LINT_DUPLICATE_COVERGROUP, errors, symbols);
      } else {
        nameMap[std::string(name)] = loc;
      }
    }
  }
}
