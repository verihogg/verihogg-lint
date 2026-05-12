#include "rules/illegal_net_datatype.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <array>

#include "main/lint_rules.h"
#include "utils/location_utils.h"
#include "utils/name_utils.h"

namespace SL = SURELOG;

namespace {

constexpr std::array kIllegalNetTypes = {
    SL::VObjectType::paNonIntType_Real,
    SL::VObjectType::paNonIntType_RealTime,
    SL::VObjectType::paNonIntType_ShortReal,
    SL::VObjectType::paString_type,
    SL::VObjectType::paChandle_type,
    SL::VObjectType::paEvent_type,
};

auto FindIllegalTypeNode(const SL::FileContent* fileContent, SL::NodeId netDecl)
    -> SL::NodeId {
  for (auto type : kIllegalNetTypes) {
    auto nodes = fileContent->sl_collect_all(netDecl, type);
    if (!nodes.empty()) {
      return nodes.front();
    }
  }
  return SL::InvalidNodeId;
}

}  // namespace

void CheckIllegalNetDatatype(const SL::FileContent* fileContent,
                             SL::ErrorContainer* errors,
                             SL::SymbolTable* symbols) {
  if (fileContent == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }
  SL::NodeId const kRoot = fileContent->getRootNode();
  if (!kRoot) {
    return;
  }

  for (SL::NodeId const kNetDecl :
       fileContent->sl_collect_all(kRoot, SL::VObjectType::paNet_declaration)) {
    SL::NodeId const kIllegal = FindIllegalTypeNode(fileContent, kNetDecl);
    if (!kIllegal) {
      continue;
    }

    ReportError(fileContent, kIllegal, ExtractName(fileContent, kNetDecl),
                verihogg_lint::LINT_ILLEGAL_NET_DATATYPE, errors, symbols);
  }
}