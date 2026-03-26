#include "rules/implicit_data_type.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/Design.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <utility>
#include <vector>

#include "main/lint_rules.h"
#include "utils/location_utils.h"
#include "utils/name_utils.h"

namespace SL = SURELOG;

using LineRange = std::pair<uint32_t, uint32_t>;

namespace {
auto Includes(const LineRange& range, uint32_t line) -> bool {
  return line >= range.first && line <= range.second;
}

auto HasExplicitType(const SL::FileContent* fileContent, SL::NodeId dataDecl)
    -> bool {
  static constexpr std::array kExplicitTypeNodes = {
      SL::VObjectType::paNet_type,
      SL::VObjectType::paData_type,
      SL::VObjectType::paInteger_atom_type,
      SL::VObjectType::paInteger_vector_type,
      SL::VObjectType::paNon_integer_type,
      SL::VObjectType::paString_type,
      SL::VObjectType::paClass_type,
      SL::VObjectType::paIntVec_TypeBit,
  };
  return std::ranges::any_of(kExplicitTypeNodes, [&](SL::VObjectType type) {
    return !fileContent->sl_collect_all(dataDecl, type).empty();
  });
}

auto CollectProceduralRanges(const SL::FileContent* fileContent,
                             SL::NodeId root) -> std::vector<LineRange> {
  static constexpr std::array kProceduralTypes = {
      SL::VObjectType::paInitial_construct,
      SL::VObjectType::paAlways_construct,
      SL::VObjectType::paFinal_construct,
  };

  std::vector<LineRange> ranges;
  for (auto procType : kProceduralTypes) {
    for (SL::NodeId const kBlock :
         fileContent->sl_collect_all(root, procType)) {
      ranges.emplace_back(fileContent->Line(kBlock),
                          fileContent->EndLine(kBlock));
    }
  }
  return ranges;
}

auto IsPhantomNode(const SL::FileContent* fileContent, SL::NodeId dataDecl,
                   const std::vector<LineRange>& ranges) -> bool {
  if (ExtractVariableName(fileContent, dataDecl).empty()) {
    return true;
  }

  uint32_t const kDeclLine = fileContent->Line(dataDecl);
  return std::ranges::any_of(ranges, [kDeclLine](const LineRange& range) {
    return Includes(range, kDeclLine);
  });
}
}  // namespace

void CheckImplicitDataTypeInDeclaration(const SL::FileContent* fileContent,
                                        SL::ErrorContainer* errors,
                                        SL::SymbolTable* symbols) {
  if (fileContent == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }
  SL::NodeId const kRoot = fileContent->getRootNode();
  if (kRoot == SL::InvalidNodeId) {
    return;
  }

  auto proceduralRanges = CollectProceduralRanges(fileContent, kRoot);

  for (SL::NodeId const kDataDecl : fileContent->sl_collect_all(
           kRoot, SL::VObjectType::paData_declaration)) {
    auto packedDims = fileContent->sl_collect_all(
        kDataDecl, SL::VObjectType::paPacked_dimension);
    if (packedDims.empty()) {
      continue;
    }

    if (HasExplicitType(fileContent, kDataDecl)) {
      continue;
    }
    if (IsPhantomNode(fileContent, kDataDecl, proceduralRanges)) {
      continue;
    }

    ReportError(fileContent, packedDims.front(),
                ExtractVariableName(fileContent, kDataDecl),
                verihogg_lint::LINT_IMPLICIT_DATA_TYPE, errors, symbols);
  }
}
