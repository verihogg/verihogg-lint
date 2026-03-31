#include "rules/duplicate_enum_literal.h"

#include <Surelog/Common/FileSystem.h>
#include <Surelog/Common/PathId.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <sstream>
#include <unordered_map>
#include <vector>

#include "main/lint_rules.h"
#include "utils/location_utils.h"

struct EnumLocation {
    SURELOG::PathId fileId;
    unsigned line;
};

void CheckDuplicateEnumLiteral(SURELOG::Design* design,
                               SURELOG::ErrorContainer* errors,
                               SURELOG::SymbolTable* symbols) {
    if (!design || !errors || !symbols) return;

    for (const auto& [fileId, fileContent] : design->getAllFileContents()) {
        if (!fileContent) continue;

        SURELOG::NodeId const root = fileContent->getRootNode();
        if (root == SURELOG::InvalidNodeId) continue;

        std::vector<SURELOG::NodeId> dataTypeNodes = fileContent->sl_collect_all(
            root,
            {SURELOG::VObjectType::paData_type},
            false
        );

        for (SURELOG::NodeId dataTypeNode : dataTypeNodes) {
            bool hasEnumDecl = false;
            SURELOG::NodeId child = fileContent->Child(dataTypeNode);
            while (child != SURELOG::InvalidNodeId) {
                if (fileContent->Type(child) == SURELOG::VObjectType::paEnum_name_declaration) {
                    hasEnumDecl = true;
                    break;
                }
                child = fileContent->Sibling(child);
            }
            if (!hasEnumDecl) continue;

            std::unordered_map<std::string, EnumLocation> enumLiterals;

            child = fileContent->Child(dataTypeNode);
            while (child != SURELOG::InvalidNodeId) {
                if (fileContent->Type(child) == SURELOG::VObjectType::paEnum_name_declaration) {
                    SURELOG::NodeId stringConst = fileContent->Child(child);
                    while (stringConst != SURELOG::InvalidNodeId &&
                           fileContent->Type(stringConst) != SURELOG::VObjectType::slStringConst) {
                        stringConst = fileContent->Sibling(stringConst);
                    }

                    if (stringConst != SURELOG::InvalidNodeId) {
                        std::string literalName = std::string(fileContent->SymName(stringConst));
                        unsigned line = fileContent->Line(child);
                        EnumLocation currentLoc{fileContent->getFileId(child), line};

                        auto it = enumLiterals.find(literalName);
                        if (it != enumLiterals.end()) {
                            const EnumLocation& first = it->second;
                            std::string_view firstPath = SURELOG::FileSystem::getInstance()->toPath(first.fileId);
                            std::ostringstream context;
                            context << "'" << literalName << "' already declared at "
                                    << firstPath << ":" << first.line;

                            ReportError(fileContent, child, context.str(),
                                        verihogg_lint::LINT_DUPLICATE_ENUM_LITERAL,
                                        errors, symbols);
                        } else {
                            enumLiterals[literalName] = currentLoc;
                        }
                    }
                }
                child = fileContent->Sibling(child);
            }
        }
    }
}
