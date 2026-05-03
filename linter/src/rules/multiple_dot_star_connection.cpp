#include "rules/multiple_dot_star_connection.h"

#include <Surelog/Common/FileSystem.h>
#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "fix/fix_it.h"
#include "main/lint_diagnostics.h"
#include "main/lint_rules.h"
#include "utils/location_utils.h"
#include "utils/name_utils.h"

namespace SL = SURELOG;

namespace {
struct DotStarResult {
  SL::NodeId secondDotStarNode;
  SL::NodeId instanceNameNode;
};

auto HasDotStarChild(const SL::FileContent* fileContent, SL::NodeId node)
    -> bool {
  return !fileContent->sl_collect_all(node, SL::VObjectType::paDOTSTAR).empty();
}

auto FindMultipleDotStarConnections(const SL::FileContent* fileContent,
                                    SL::NodeId instNode)
    -> std::optional<DotStarResult> {
  SL::NodeId const kHierInst = fileContent->sl_collect(
      instNode, SL::VObjectType::paHierarchical_instance);
  if (kHierInst == SL::InvalidNodeId) {
    return std::nullopt;
  }

  SL::NodeId instanceNameNode = {};
  SL::NodeId const kNameOfInst =
      fileContent->sl_collect(kHierInst, SL::VObjectType::paName_of_instance);
  if (kNameOfInst) {
    instanceNameNode = fileContent->Child(kNameOfInst);
  }

  SL::NodeId const kPortList = fileContent->sl_collect(
      kHierInst, SL::VObjectType::paList_of_port_connections);
  if (kPortList == SL::InvalidNodeId) {
    return std::nullopt;
  }

  int dotStarCount = 0;
  for (SL::NodeId child = fileContent->Child(kPortList); child;
       child = fileContent->Sibling(child)) {
    if (fileContent->Type(child) != SL::VObjectType::paNamed_port_connection) {
      continue;
    }
    if (!HasDotStarChild(fileContent, child)) {
      continue;
    }

    if (++dotStarCount == 2) {
      SL::NodeId const kDotStarNode =
          fileContent->sl_collect(child, SL::VObjectType::paDOTSTAR);
      return DotStarResult{
          .secondDotStarNode = kDotStarNode ? kDotStarNode : child,
          .instanceNameNode = instanceNameNode};
    }
  }

  return std::nullopt;
}
}  // namespace

auto CheckMultipleDotStarConnectionsFixable(const SL::FileContent* fileContent,
                                            SL::ErrorContainer* errors,
                                            SL::SymbolTable* symbols,
                                            FixSourceManager& /*sm*/)
    -> std::vector<LintDiagnostic> {
  std::vector<LintDiagnostic> diags;

  if (fileContent == nullptr || errors == nullptr || symbols == nullptr) {
    return diags;
  }

  SL::NodeId const kRoot = fileContent->getRootNode();
  if (kRoot == SL::InvalidNodeId) {
    return diags;
  }

  const std::string filepath = GetFixFilepath(fileContent);
  if (filepath.empty()) {
    return diags;
  }

  std::vector<std::string> srcLines;
  SL::FileSystem* const fileSystem = SL::FileSystem::getInstance();
  if (fileSystem != nullptr) {
    (void)fileSystem->readLines(fileContent->getFileId(), srcLines);
  }

  for (SL::NodeId const kInst : fileContent->sl_collect_all(
           kRoot, SL::VObjectType::paModule_instantiation)) {
    auto result = FindMultipleDotStarConnections(fileContent, kInst);
    if (!result) {
      continue;
    }

    std::string_view const kInstanceName =
        result->instanceNameNode
            ? ExtractName(fileContent, result->instanceNameNode, "unknown")
            : "unknown";

    ReportError(fileContent, result->secondDotStarNode, kInstanceName,
                verihogg_lint::LINT_MULTIPLE_DOT_STAR_CONNECTIONS, errors,
                symbols);

    const unsigned line = fileContent->Line(result->secondDotStarNode);
    const unsigned col = GetColumnSafe(fileContent, result->secondDotStarNode);
    if (line == 0 || col < 2 || line > srcLines.size()) {
      continue;
    }

    const std::string& srcLine = srcLines.at(line - 1);

    // Scan backward from just before '.' to find the preceding ','
    size_t searchIdx = col - 2;  // 0-based index of char before '.'
    while (searchIdx > 0 && srcLine.at(searchIdx) != ',') {
      --searchIdx;
    }
    if (srcLine.at(searchIdx) != ',') {
      continue;
    }

    const auto commaCol = static_cast<unsigned>(searchIdx + 1);  // 1-based

    const FixRange remove_range{
        .begin =
            FixLocation{.filename = filepath, .line = line, .col = commaCol},
        .end = FixLocation{.filename = filepath, .line = line, .col = col + 2},
    };

    LintDiagnostic d;
    d.filepath = filepath;
    d.line = line;
    d.col = col;
    d.message = std::string(kInstanceName) + ": remove duplicate .*";

    try {
      d.fixes.push_back(
          FixIt::Remove(remove_range, "remove duplicate .* from port list"));
    } catch (const std::exception& e) {
      std::cerr << "autofix: cannot create fix at " << filepath << ":" << line
                << ":" << col << ": " << e.what() << "\n";
      diags.push_back(std::move(d));
      continue;
    }

    diags.push_back(std::move(d));
  }

  return diags;
}
