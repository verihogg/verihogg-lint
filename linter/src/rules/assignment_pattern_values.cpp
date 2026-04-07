#include "rules/assignment_pattern_values.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

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

auto ParseRangeWidth(const SL::FileContent* fc, SL::NodeId dimNode) -> int32_t {
  SL::NodeId const kRange =
      FindChildOfType(fc, dimNode, SL::VObjectType::paConstant_range);
  if (!kRange) return -1;

  SL::NodeId const kLeft = fc->Child(kRange);
  if (!kLeft) return -1;
  SL::NodeId const kRight = fc->Sibling(kLeft);
  if (!kRight) return -1;

  auto getInt = [&](SL::NodeId expr) -> std::optional<int64_t> {
    for (const auto& kC :
         fc->sl_collect_all(expr, SL::VObjectType::slIntConst)) {
      std::string_view sv = fc->SymName(kC);
      try {
        return std::stoll(std::string(sv));
      } catch (...) {
      }
    }
    return std::nullopt;
  };

  auto left = getInt(kLeft);
  auto right = getInt(kRight);
  if (!left || !right) return -1;

  int64_t hi = *left, lo = *right;
  if (hi < lo) std::swap(hi, lo);
  return static_cast<int32_t>(hi - lo + 1);
}

auto CountStructMembers(const SL::FileContent* fc, SL::NodeId structNode)
    -> int32_t {
  SL::NodeId const kDataType = fc->Parent(structNode);
  if (!kDataType) return -1;

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
    auto structs =
        fc->sl_collect_all(kTypeDecl, SL::VObjectType::paStruct_union);
    if (structs.empty()) continue;

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

auto ExpectedCount(const SL::FileContent* fc, SL::NodeId moduleRoot,
                   SL::NodeId fileRoot, std::string_view varName) -> int32_t {
  auto checkDecl = [&](SL::NodeId decl, SL::VObjectType assignType) -> int32_t {
    bool found = false;
    for (const auto& kAssign : fc->sl_collect_all(decl, assignType)) {
      SL::NodeId const kName = fc->Child(kAssign);
      if (kName && fc->Type(kName) == SL::VObjectType::slStringConst &&
          fc->SymName(kName) == varName) {
        found = true;
        break;
      }
    }
    if (!found) return -1;

    auto structs = fc->sl_collect_all(decl, SL::VObjectType::paStruct_union);
    if (!structs.empty()) return CountStructMembers(fc, structs.front());

    auto dims = fc->sl_collect_all(decl, SL::VObjectType::paPacked_dimension);
    if (!dims.empty()) return ParseRangeWidth(fc, dims.front());

    // typedef — спускаемся по Child до первого slStringConst (имя типа)
    for (SL::NodeId cur = fc->Child(decl); cur; cur = fc->Child(cur)) {
      if (fc->Type(cur) ==
          SL::VObjectType::paList_of_variable_decl_assignments) {
        break;
      }
      if (fc->Type(cur) == SL::VObjectType::slStringConst) {
        SL::NodeId const kStruct =
            FindStructByTypeName(fc, fileRoot, fc->SymName(cur));
        if (kStruct) return CountStructMembers(fc, kStruct);
        break;
      }
    }

    return -1;
  };

  for (const auto& kDecl :
       fc->sl_collect_all(moduleRoot, SL::VObjectType::paNet_declaration)) {
    int32_t const kCount =
        checkDecl(kDecl, SL::VObjectType::paNet_decl_assignment);
    if (kCount >= 0) return kCount;
  }

  for (const auto& kDecl :
       fc->sl_collect_all(moduleRoot, SL::VObjectType::paData_declaration)) {
    int32_t const kCount =
        checkDecl(kDecl, SL::VObjectType::paVariable_decl_assignment);
    if (kCount >= 0) return kCount;
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
  SL::NodeId const kRoot = fileContent->getRootNode();
  if (!kRoot) return;

  for (const auto& kPat : fileContent->sl_collect_all(
           kRoot, SL::VObjectType::paAssignment_pattern)) {
    if (!kPat) continue;

    if (!fileContent
             ->sl_collect_all(kPat, SL::VObjectType::paStructure_pattern_key)
             .empty()) {
      continue;
    }

    std::string_view const kVarName = FindDirectRhsLhsName(fileContent, kPat);
    if (kVarName == "<unknown>" || kVarName == "<indexed>") continue;

    SL::NodeId const kModule = FindEnclosingModule(fileContent, kPat);
    if (!kModule) continue;

    int32_t const kExpected =
        ExpectedCount(fileContent, kModule, kRoot, kVarName);
    if (kExpected < 0) continue;

    int32_t const kActual = CountChildren(fileContent, kPat);
    if (kActual == kExpected) continue;

    std::string sym = "Expecting " + std::to_string(kExpected) +
                      " value(s), found " + std::to_string(kActual) + " for '" +
                      std::string(kVarName) + "'";
    ReportError(fileContent, kPat, sym,
                verihogg_lint::LINT_ASSIGNMENT_PATTERN_VALUES, errors, symbols);
  }
}
