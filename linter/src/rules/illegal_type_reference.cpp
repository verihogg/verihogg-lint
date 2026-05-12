#include "rules/illegal_type_reference.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <algorithm>
#include <array>

#include "main/lint_rules.h"
#include "utils/ast_utils.h"
#include "utils/location_utils.h"
#include "utils/name_utils.h"

namespace SL = SURELOG;

namespace {

constexpr std::array kIllegalPackedTypes = {
    SL::VObjectType::paNonIntType_Real,
    SL::VObjectType::paNonIntType_RealTime,
    SL::VObjectType::paNonIntType_ShortReal,
    SL::VObjectType::paString_type,
    SL::VObjectType::paChandle_type,
};

auto FindIllegalBaseChild(const SL::FileContent* fc, SL::NodeId dataType)
    -> SL::NodeId {
  for (SL::NodeId c = fc->Child(dataType); c; c = fc->Sibling(c)) {
    auto t = fc->Type(c);
    if (std::ranges::find(kIllegalPackedTypes, t) !=
        kIllegalPackedTypes.end()) {
      return c;
    }
  }
  return SL::InvalidNodeId;
}

}  // namespace

void CheckIllegalTypeReference(const SL::FileContent* fileContent,
                               SL::ErrorContainer* errors,
                               SL::SymbolTable* symbols) {
  if (fileContent == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }
  SL::NodeId const kRoot = fileContent->getRootNode();
  if (!kRoot) {
    return;
  }

  for (SL::NodeId const kMember : fileContent->sl_collect_all(
           kRoot, SL::VObjectType::paStruct_union_member)) {
    SL::NodeId const kOuter = fileContent->Parent(kMember);
    if (!kOuter || !FindChildOfType(fileContent, kOuter,
                                    SL::VObjectType::paPacked_keyword)) {
      continue;
    }

    SL::NodeId const kDtOrVoid = FindChildOfType(
        fileContent, kMember, SL::VObjectType::paData_type_or_void);
    if (!kDtOrVoid) {
      continue;
    }
    SL::NodeId const kDataType =
        FindChildOfType(fileContent, kDtOrVoid, SL::VObjectType::paData_type);
    if (!kDataType) {
      continue;
    }

    SL::NodeId const kIllegal = FindIllegalBaseChild(fileContent, kDataType);
    if (!kIllegal) {
      continue;
    }

    ReportError(fileContent, kIllegal, ExtractName(fileContent, kMember),
                verihogg_lint::LINT_ILLEGAL_TYPE_REFERENCE, errors, symbols);
  }
}