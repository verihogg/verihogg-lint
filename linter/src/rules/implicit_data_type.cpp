#include "rules/implicit_data_type.h"

#include <Surelog/Design/Design.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <algorithm>

#include "utils/location_utils.h"
#include "utils/name_utils.h"

namespace SL = SURELOG;

using LineRange = std::pair<uint32_t, uint32_t>;

static auto Includes(const LineRange& range, uint32_t line) -> bool {
  return line >= range.first && line <= range.second;
}

static auto HasExplicitType(const SL::FileContent* fileContent,
                            SL::NodeId dataDecl) -> bool {
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

static auto CollectProceduralRanges(const SL::FileContent* fileContent,
                                    SL::NodeId root) -> std::vector<LineRange> {
  static constexpr std::array kProceduralTypes = {
      SL::VObjectType::paInitial_construct,
      SL::VObjectType::paAlways_construct,
      SL::VObjectType::paFinal_construct,
  };

  std::vector<LineRange> ranges;
  for (auto procType : kProceduralTypes) {
    for (SL::NodeId block : fileContent->sl_collect_all(root, procType)) {
      ranges.emplace_back(fileContent->Line(block),
                          fileContent->EndLine(block));
    }
  }
  return ranges;
}

static auto IsPhantomNode(const SL::FileContent* fileContent,
                          SL::NodeId dataDecl,
                          const std::vector<LineRange>& ranges) -> bool {
  if (ExtractVariableName(fileContent, dataDecl).empty()) {
    return true;
  }

  uint32_t declLine = fileContent->Line(dataDecl);
  return std::ranges::any_of(ranges, [declLine](const LineRange& range) {
    return Includes(range, declLine);
  });
}

void CheckImplicitDataTypeInDeclaration(const SL::FileContent* fileContent,
                                        SL::ErrorContainer* errors,
                                        SL::SymbolTable* symbols) {
  if (fileContent == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }
  SL::NodeId root = fileContent->getRootNode();
  if (root == SL::InvalidNodeId) {
    return;
  }

  auto proceduralRanges = CollectProceduralRanges(fileContent, root);

  for (SL::NodeId dataDecl :
       fileContent->sl_collect_all(root, SL::VObjectType::paData_declaration)) {
    auto packedDims = fileContent->sl_collect_all(
        dataDecl, SL::VObjectType::paPacked_dimension);
    if (packedDims.empty()) {
      continue;
    }

    if (HasExplicitType(fileContent, dataDecl)) {
      continue;
    }
    if (IsPhantomNode(fileContent, dataDecl, proceduralRanges)) {
      continue;
    }

    ReportError(fileContent, packedDims.front(),
                ExtractVariableName(fileContent, dataDecl),
                SL::ErrorDefinition::LINT_IMPLICIT_DATA_TYPE, errors, symbols);
  }
}
