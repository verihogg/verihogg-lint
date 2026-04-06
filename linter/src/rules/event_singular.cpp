#include "rules/event_singular.h"

#include <Surelog/Design/FileContent.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <sstream>
#include <vector>

#include "main/lint_rules.h"
#include "utils/location_utils.h"

void CheckEventSingular(SURELOG::Design* design,
                        SURELOG::ErrorContainer* errors,
                        SURELOG::SymbolTable* symbols) {
  if (!design || !errors || !symbols) return;

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

      bool isArray =
          !fileContent
               ->sl_collect_all(dataDecl,
                                {SURELOG::VObjectType::paPacked_dimension,
                                 SURELOG::VObjectType::paUnpacked_dimension,
                                 SURELOG::VObjectType::paQueue_dimension,
                                 SURELOG::VObjectType::paAssociative_dimension},
                                false)
               .empty();

      std::vector<std::string> eventNames;
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
            eventNames.push_back(std::string(fileContent->SymName(identifier)));
            break;
          }
          parent = fileContent->Parent(parent);
        }
      }

      if (isArray || eventNames.size() > 1) {
        std::ostringstream namesStr;
        for (size_t i = 0; i < eventNames.size(); ++i) {
          if (i > 0) namesStr << ", ";
          namesStr << "'" << eventNames[i] << "'";
        }
        std::string context = namesStr.str();

        ReportError(fileContent, dataDecl, context,
                    verihogg_lint::LINT_EVENT_SINGULAR, errors, symbols);
      }
    }
  }
}
