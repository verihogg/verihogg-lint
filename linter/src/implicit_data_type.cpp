#include "implicit_data_type.h"

#include <cstdint>
#include <string>

#include "Surelog/Design/Design.h"
#include "Surelog/Design/FileContent.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"
#include "Surelog/SourceCompile/VObjectTypes.h"
#include "linter_utils.h"

using namespace SURELOG;

namespace Analyzer {

std::string findVarName(const FileContent* fC, NodeId dataDecl) {
  return extractVariableName(fC, dataDecl);
}

bool hasExplicitType(const FileContent* fC, NodeId dataDecl) {
  static const VObjectType typeNodes[] = {
      VObjectType::paNet_type,          VObjectType::paData_type,
      VObjectType::paInteger_atom_type, VObjectType::paInteger_vector_type,
      VObjectType::paNon_integer_type,  VObjectType::paString_type,
      VObjectType::paClass_type,        VObjectType::paIntVec_TypeBit};

  for (auto t : typeNodes) {
    if (!fC->sl_collect_all(dataDecl, t).empty()) return true;
  }
  return false;
}

static std::vector<std::pair<uint32_t, uint32_t>> collectProceduralRanges(
    const FileContent* fC, NodeId root) {
  std::vector<std::pair<uint32_t, uint32_t>> ranges;

  static const VObjectType proceduralTypes[] = {
      VObjectType::paInitial_construct,
      VObjectType::paAlways_construct,
      VObjectType::paFinal_construct,
  };

  for (auto procType : proceduralTypes) {
    for (NodeId block : fC->sl_collect_all(root, procType)) {
      uint32_t startLine = fC->Line(block);
      uint32_t endLine = fC->EndLine(block);
      ranges.push_back({startLine, endLine});
    }
  }
  return ranges;
}

static bool isPhantomNode(
    const FileContent* fC, NodeId dataDecl,
    const std::vector<std::pair<uint32_t, uint32_t>>& ranges) {
  std::string varName = findVarName(fC, dataDecl);
  if (varName.empty()) return true;

  uint32_t declLine = fC->Line(dataDecl);
  for (auto& [startLine, endLine] : ranges) {
    if (declLine >= startLine && declLine <= endLine) return true;
  }
  return false;
}

void checkImplicitDataTypeInDeclaration(const FileContent* fC,
                                        ErrorContainer* errors,
                                        SymbolTable* symbols) {
  NodeId root = fC->getRootNode();

  auto proceduralRanges = collectProceduralRanges(fC, root);

  auto dataDecls = fC->sl_collect_all(root, VObjectType::paData_declaration);

  for (NodeId dataDecl : dataDecls) {
    auto packedDims =
        fC->sl_collect_all(dataDecl, VObjectType::paPacked_dimension);
    if (packedDims.empty()) continue;

    if (hasExplicitType(fC, dataDecl)) continue;
    if (isPhantomNode(fC, dataDecl, proceduralRanges)) continue;

    std::string varName = findVarName(fC, dataDecl);
    NodeId where = packedDims.front();

    reportError(fC, where, varName, ErrorDefinition::LINT_IMPLICIT_DATA_TYPE,
                errors, symbols);
  }
}
}  // namespace Analyzer
