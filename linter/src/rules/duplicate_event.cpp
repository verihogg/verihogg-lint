#include "rules/duplicate_event.h"

#include <Surelog/Common/FileSystem.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <sstream>
#include <unordered_map>
#include <utility>

#include "main/lint_rules.h"
#include "utils/location_utils.h"

struct Location {
  SURELOG::PathId fileId;
  uint32_t line;
};

struct ScopeId {
  SURELOG::PathId fileId;
  SURELOG::NodeId scopeNode;

  bool operator==(const ScopeId& other) const {
    return fileId == other.fileId && scopeNode == other.scopeNode;
  }
};

struct ScopeIdHash {
  size_t operator()(const ScopeId& id) const {
    SURELOG::PathIdHasher pathHasher;
    SURELOG::NodeIdHasher nodeHasher;
    return pathHasher(id.fileId) ^ (nodeHasher(id.scopeNode) << 1);
  }
};

void CheckDuplicateEvents(SURELOG::Design* design,
                          SURELOG::ErrorContainer* errors,
                          SURELOG::SymbolTable* symbols) {
  if (!design || !errors || !symbols) return;

  std::unordered_map<ScopeId, std::unordered_map<std::string, Location>,
                     ScopeIdHash>
      scopeEvents;

  for (const auto& [fileId, fileContent] : design->getAllFileContents()) {
    if (!fileContent) continue;

    SURELOG::NodeId const root = fileContent->getRootNode();
    if (root == SURELOG::InvalidNodeId) continue;

    for (SURELOG::NodeId eventNode : fileContent->sl_collect_all(
             root, SURELOG::VObjectType::paEvent_type)) {
      SURELOG::NodeId dataDecl = eventNode;
      while (dataDecl != SURELOG::InvalidNodeId &&
             fileContent->Type(dataDecl) !=
                 SURELOG::VObjectType::paData_declaration) {
        dataDecl = fileContent->Parent(dataDecl);
      }
      if (dataDecl == SURELOG::InvalidNodeId) continue;

      SURELOG::NodeId scopeNode = dataDecl;
      while (scopeNode != SURELOG::InvalidNodeId) {
        SURELOG::VObjectType type = fileContent->Type(scopeNode);
        if (type == SURELOG::VObjectType::paModule_declaration ||
            type == SURELOG::VObjectType::paClass_declaration ||
            type == SURELOG::VObjectType::paPackage_declaration ||
            type == SURELOG::VObjectType::paInterface_declaration ||
            type == SURELOG::VObjectType::paProgram_declaration ||
            type == SURELOG::VObjectType::paChecker_declaration) {
          break;
        }
        scopeNode = fileContent->Parent(scopeNode);
      }
      if (scopeNode == SURELOG::InvalidNodeId) {
        scopeNode = root;
      }

      ScopeId scopeKey{fileContent->getFileId(scopeNode), scopeNode};

      for (SURELOG::NodeId identifier : fileContent->sl_collect_all(
               dataDecl, SURELOG::VObjectType::slStringConst)) {
        SURELOG::NodeId parent = fileContent->Parent(identifier);
        while (parent != SURELOG::InvalidNodeId) {
          SURELOG::VObjectType type = fileContent->Type(parent);
          if (type == SURELOG::VObjectType::paData_type ||
              type == SURELOG::VObjectType::paType_declaration) {
            break;
          }
          if (parent == dataDecl) {
            std::string eventName(fileContent->SymName(identifier));
            unsigned line = fileContent->Line(dataDecl);
            Location currentLoc{fileContent->getFileId(dataDecl), line};

            auto& eventsInScope = scopeEvents[scopeKey];
            auto it = eventsInScope.find(eventName);
            if (it != eventsInScope.end()) {
              const Location& first = it->second;
              std::string_view firstPath =
                  SURELOG::FileSystem::getInstance()->toPath(first.fileId);
              std::ostringstream context;
              context << "'" << eventName << "' already declared at "
                      << firstPath << ":" << first.line;

              ReportError(fileContent, dataDecl, context.str(),
                          verihogg_lint::LINT_DUPLICATE_EVENT, errors, symbols);
            } else {
              eventsInScope.emplace(eventName, currentLoc);
            }
            break;
          }
          parent = fileContent->Parent(parent);
        }
      }
    }
  }
}
