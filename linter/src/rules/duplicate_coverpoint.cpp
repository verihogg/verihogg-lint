#include "rules/duplicate_coverpoint.h"

#include <Surelog/Common/FileSystem.h>
#include <Surelog/Common/PathId.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <map>
#include <sstream>
#include <unordered_map>
#include <vector>

#include "main/lint_rules.h"
#include "utils/location_utils.h"

namespace {
    std::string_view getCoverpointLabel(const SURELOG::FileContent* fc,
                                        SURELOG::NodeId cpNode) {
        SURELOG::NodeId child = fc->Child(cpNode);
        while (child != SURELOG::InvalidNodeId) {
            SURELOG::VObjectType type = fc->Type(child);
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

    SURELOG::NodeId findCovergroupParent(const SURELOG::FileContent* fc,
                                         SURELOG::NodeId node) {
        while (node != SURELOG::InvalidNodeId) {
            SURELOG::VObjectType type = fc->Type(node);
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
    if (!design || !errors || !symbols) return;

    struct Location {
        SURELOG::PathId fileId;
        unsigned line;
    };

    SURELOG::VObjectTypeUnorderedSet cpTypes = {
        SURELOG::VObjectType::paCover_point,
        SURELOG::VObjectType::paCOVERPOINT
    };

    using Key = std::pair<const SURELOG::FileContent*, SURELOG::NodeId>;
    std::map<Key, std::unordered_map<std::string, Location>> covergroupLabels;

    for (const auto& [fileId, fileContent] : design->getAllFileContents()) {
        if (!fileContent) continue;

        std::vector<SURELOG::NodeId> cpNodes = fileContent->sl_collect_all(
            fileContent->getRootNode(), cpTypes, false);

        for (SURELOG::NodeId cpNode : cpNodes) {
            std::string_view label = getCoverpointLabel(fileContent, cpNode);
            if (label.empty()) continue;

            SURELOG::NodeId covergroupNode = findCovergroupParent(fileContent, cpNode);
            if (covergroupNode == SURELOG::InvalidNodeId) continue;

            Key key = std::make_pair(fileContent, covergroupNode);
            Location loc{ fileContent->getFileId(cpNode), fileContent->Line(cpNode) };

            auto& labelMap = covergroupLabels[key];
            auto it = labelMap.find(std::string(label));
            if (it != labelMap.end()) {
                const Location& first = it->second;
                std::string_view firstPath = SURELOG::FileSystem::getInstance()->toPath(first.fileId);
                std::string_view secondPath = SURELOG::FileSystem::getInstance()->toPath(loc.fileId);
                std::ostringstream context;
                context << "'" << label << "' (first at "
                        << firstPath << ":" << first.line
                        << ", duplicate at "
                        << secondPath << ":" << loc.line << ")";

                ReportError(fileContent, cpNode, context.str(),
                            verihogg_lint::LINT_DUPLICATE_COVERPOINT,
                            errors, symbols);
            } else {
                labelMap[std::string(label)] = loc;
            }
        }
    }
}
