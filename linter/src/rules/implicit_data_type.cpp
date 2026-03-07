#include "rules/implicit_data_type.h"

#include <algorithm>

#include "Surelog/Design/Design.h"
#include "Surelog/Design/FileContent.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"
#include "Surelog/SourceCompile/VObjectTypes.h"
#include "utils/location_utils.h"
#include "utils/name_utils.h"

using namespace SURELOG;

struct LineRange {
  uint32_t start;
  uint32_t end;
  bool includes(uint32_t line) const { return line >= start && line <= end; }
};

static bool hasExplicitType(const FileContent* fC, NodeId dataDecl) {
  static constexpr std::array kExplicitTypeNodes = {
      VObjectType::paNet_type,          VObjectType::paData_type,
      VObjectType::paInteger_atom_type, VObjectType::paInteger_vector_type,
      VObjectType::paNon_integer_type,  VObjectType::paString_type,
      VObjectType::paClass_type,        VObjectType::paIntVec_TypeBit,
  };
  return std::ranges::any_of(kExplicitTypeNodes, [&](VObjectType t) {
    return !fC->sl_collect_all(dataDecl, t).empty();
  });
}

static std::vector<LineRange> collectProceduralRanges(const FileContent* fC,
                                                      NodeId root) {
  static constexpr std::array kProceduralTypes = {
      VObjectType::paInitial_construct,
      VObjectType::paAlways_construct,
      VObjectType::paFinal_construct,
  };

  std::vector<LineRange> ranges;
  for (auto procType : kProceduralTypes) {
    for (NodeId block : fC->sl_collect_all(root, procType)) {
      ranges.push_back({fC->Line(block), fC->EndLine(block)});
    }
  }
  return ranges;
}

static bool isPhantomNode(const FileContent* fC, NodeId dataDecl,
                          const std::vector<LineRange>& ranges) {
  if (extractVariableName(fC, dataDecl).empty()) return true;

  uint32_t declLine = fC->Line(dataDecl);
  return std::ranges::any_of(
      ranges, [declLine](const LineRange& r) { return r.includes(declLine); });
}

void checkImplicitDataTypeInDeclaration(const FileContent* fC,
                                        ErrorContainer* errors,
                                        SymbolTable* symbols) {
  if (!fC || !errors || !symbols) return;
  NodeId root = fC->getRootNode();
  if (!root) return;

  auto proceduralRanges = collectProceduralRanges(fC, root);

  for (NodeId dataDecl :
       fC->sl_collect_all(root, VObjectType::paData_declaration)) {
    auto packedDims =
        fC->sl_collect_all(dataDecl, VObjectType::paPacked_dimension);
    if (packedDims.empty()) continue;

    if (hasExplicitType(fC, dataDecl)) continue;
    if (isPhantomNode(fC, dataDecl, proceduralRanges)) continue;

    reportError(fC, packedDims.front(), extractVariableName(fC, dataDecl),
                ErrorDefinition::LINT_IMPLICIT_DATA_TYPE, errors, symbols);
  }
}
