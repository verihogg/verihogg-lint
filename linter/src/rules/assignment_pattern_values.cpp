#include "rules/assignment_pattern_values.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

#include "main/lint_rules.h"
#include "utils/ast_utils.h"
#include "utils/location_utils.h"
#include "utils/name_utils.h"

namespace SL = SURELOG;

namespace {

auto CountChildren(const SL::FileContent* fc, SL::NodeId node) -> int32_t {
  int32_t count = 0;
  for (SL::NodeId ch = fc->Child(node); ch; ch = fc->Sibling(ch)) {
    ++count;
  }
  return count;
}

auto TryParseInt(std::string_view sv) -> std::optional<int64_t> {
  if (sv.empty()) {
    return std::nullopt;
  }
  constexpr int64_t kDecimalBase = 10;
  int64_t result = 0;
  for (const char c : sv) {
    if (c < '0' || c > '9') {
      return std::nullopt;
    }
    result = result * kDecimalBase + (c - '0');
  }
  return result;
}

auto ParseRangeWidth(const SL::FileContent* fc, SL::NodeId dimNode) -> int32_t {
  SL::NodeId range =
      FindChildOfType(fc, dimNode, SL::VObjectType::paConstant_range);
  if (!range) {
    const auto ranges =
        fc->sl_collect_all(dimNode, SL::VObjectType::paConstant_range);
    if (ranges.empty()) {
      return -1;
    }
    range = ranges.front();
  }

  const SL::NodeId kLeft = fc->Child(range);
  if (!kLeft) {
    return -1;
  }
  const SL::NodeId kRight = fc->Sibling(kLeft);
  if (!kRight) {
    return -1;
  }

  auto getInt = [&](SL::NodeId expr) -> std::optional<int64_t> {
    for (const auto& kC :
         fc->sl_collect_all(expr, SL::VObjectType::slIntConst)) {
      const auto val = TryParseInt(fc->SymName(kC));
      if (val) {
        return val;
      }
    }
    return std::nullopt;
  };

  const auto left = getInt(kLeft);
  const auto right = getInt(kRight);
  if (!left || !right) {
    return -1;
  }

  int64_t hi = *left;
  int64_t lo = *right;
  if (hi < lo) {
    std::swap(hi, lo);
  }
  return static_cast<int32_t>(hi - lo + 1);
}

auto CountStructMembers(const SL::FileContent* fc,
                        SL::NodeId structNode) -> int32_t {
  const SL::NodeId kDataType = fc->Parent(structNode);
  if (!kDataType) {
    return -1;
  }

  int32_t count = 0;
  for (const auto& kMember :
       fc->sl_collect_all(kDataType, SL::VObjectType::paStruct_union_member)) {
    count += static_cast<int32_t>(
        fc->sl_collect_all(kMember, SL::VObjectType::paVariable_decl_assignment)
            .size());
  }
  return count > 0 ? count : -1;
}

auto FindStructByTypeName(const SL::FileContent* fc, SL::NodeId root,
                          std::string_view typeName) -> SL::NodeId {
  for (const auto& kTypeDecl :
       fc->sl_collect_all(root, SL::VObjectType::paType_declaration)) {
    const auto structs =
        fc->sl_collect_all(kTypeDecl, SL::VObjectType::paStruct_union);
    if (structs.empty()) {
      continue;
    }

    for (SL::NodeId child = fc->Child(kTypeDecl); child;
         child = fc->Sibling(child)) {
      if (fc->Type(child) == SL::VObjectType::slStringConst &&
          fc->SymName(child) == typeName) {
        return structs.front();
      }
    }
  }
  return SL::InvalidNodeId;
}

auto ParseDimSize(const SL::FileContent* fc, SL::NodeId dimNode) -> int32_t {
  const int32_t kRange = ParseRangeWidth(fc, dimNode);
  if (kRange >= 0) {
    return kRange;
  }
  for (const auto& kI :
       fc->sl_collect_all(dimNode, SL::VObjectType::slIntConst)) {
    const auto val = TryParseInt(fc->SymName(kI));
    if (val) {
      return static_cast<int32_t>(*val);
    }
  }
  return -1;
}

auto FindUnpackedDim(const SL::FileContent* fc,
                     SL::NodeId nameNode) -> SL::NodeId {
  const SL::NodeId kUnpacked =
      FindSiblingOfType(fc, nameNode, SL::VObjectType::paUnpacked_dimension);
  if (kUnpacked) {
    return kUnpacked;
  }
  return FindSiblingOfType(fc, nameNode, SL::VObjectType::paVariable_dimension);
}

auto ExpectedCount(const SL::FileContent* fc, SL::NodeId moduleRoot,
                   std::string_view varName) -> int32_t {
  const SL::NodeId kFileRoot = fc->getRootNode();

  auto checkDecl = [&](SL::NodeId decl, SL::VObjectType assignType) -> int32_t {
    SL::NodeId matchedName = SL::InvalidNodeId;
    for (const auto& kAssign : fc->sl_collect_all(decl, assignType)) {
      const SL::NodeId kName = fc->Child(kAssign);
      if (kName && fc->Type(kName) == SL::VObjectType::slStringConst &&
          fc->SymName(kName) == varName) {
        matchedName = kName;
        break;
      }
    }
    if (!matchedName) {
      return -1;
    }

    const SL::NodeId kUnpackedDim = FindUnpackedDim(fc, matchedName);
    if (kUnpackedDim) {
      return ParseDimSize(fc, kUnpackedDim);
    }

    const auto structs =
        fc->sl_collect_all(decl, SL::VObjectType::paStruct_union);
    if (!structs.empty()) {
      return CountStructMembers(fc, structs.front());
    }

    const auto dims =
        fc->sl_collect_all(decl, SL::VObjectType::paPacked_dimension);
    if (!dims.empty()) {
      return ParseRangeWidth(fc, dims.front());
    }

    // typedef — спускаемся по Child до первого slStringConst (имя типа)
    for (SL::NodeId cur = fc->Child(decl); cur; cur = fc->Child(cur)) {
      if (fc->Type(cur) ==
          SL::VObjectType::paList_of_variable_decl_assignments) {
        break;
      }
      if (fc->Type(cur) == SL::VObjectType::slStringConst) {
        const SL::NodeId kStruct =
            FindStructByTypeName(fc, kFileRoot, fc->SymName(cur));
        if (kStruct) {
          return CountStructMembers(fc, kStruct);
        }
        break;
      }
    }

    return -1;
  };

  for (const auto& kDecl :
       fc->sl_collect_all(moduleRoot, SL::VObjectType::paNet_declaration)) {
    const int32_t kCount =
        checkDecl(kDecl, SL::VObjectType::paNet_decl_assignment);
    if (kCount >= 0) {
      return kCount;
    }
  }

  for (const auto& kDecl :
       fc->sl_collect_all(moduleRoot, SL::VObjectType::paData_declaration)) {
    const int32_t kCount =
        checkDecl(kDecl, SL::VObjectType::paVariable_decl_assignment);
    if (kCount >= 0) {
      return kCount;
    }
  }

  return -1;
}

}  // namespace

void CheckAssignmentPatternValues(const SL::FileContent* fileContent,
                                  SL::ErrorContainer* errors,
                                  SL::SymbolTable* symbols) {
  if (fileContent == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }
  const SL::NodeId kRoot = fileContent->getRootNode();
  if (!kRoot) {
    return;
  }

  for (const auto& kPat : fileContent->sl_collect_all(
           kRoot, SL::VObjectType::paAssignment_pattern)) {
    if (!kPat) {
      continue;
    }

    if (!fileContent
             ->sl_collect_all(kPat, SL::VObjectType::paStructure_pattern_key)
             .empty()) {
      continue;
    }

    const std::string_view kVarName = FindDirectRhsLhsName(fileContent, kPat);
    if (kVarName == "<unknown>" || kVarName == "<indexed>") {
      continue;
    }

    const SL::NodeId kModule = FindEnclosingModule(fileContent, kPat);
    if (!kModule) {
      continue;
    }

    const int32_t kExpected = ExpectedCount(fileContent, kModule, kVarName);
    if (kExpected < 0) {
      continue;
    }

    const int32_t kActual = CountChildren(fileContent, kPat);
    if (kActual == kExpected) {
      continue;
    }

    const std::string sym = "Expecting " + std::to_string(kExpected) +
                            " value(s), found " + std::to_string(kActual) +
                            " for '" + std::string(kVarName) + "'";
    ReportError(fileContent, kPat, sym,
                verihogg_lint::LINT_ASSIGNMENT_PATTERN_VALUES, errors, symbols);
  }
}