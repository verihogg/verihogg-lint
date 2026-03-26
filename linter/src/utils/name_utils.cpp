#include "utils/name_utils.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <string_view>
#include <unordered_set>

namespace SL = SURELOG;

auto ExtractName(const SL::FileContent* fileContent, SL::NodeId node,
                 const std::string_view& defaultName) -> std::string_view {
  if (fileContent == nullptr || !node) {
    return defaultName;
  }

  auto stringNodes =
      fileContent->sl_collect_all(node, SL::VObjectType::slStringConst);
  for (SL::NodeId const kNameNode : stringNodes) {
    if (kNameNode &&
        fileContent->Type(kNameNode) == SL::VObjectType::slStringConst) {
      return fileContent->SymName(kNameNode);
    }
  }

  SL::NodeId const kChild = fileContent->Child(node);
  if (kChild && fileContent->Type(kChild) == SL::VObjectType::slStringConst) {
    return fileContent->SymName(kChild);
  }

  return defaultName;
}

auto FindForLoopVariableName(const SL::FileContent* fileContent,
                             SL::NodeId forNode) -> std::string_view {
  if (fileContent == nullptr || !forNode) {
    return "<unknown>";
  }

  SL::NodeId forInit = SL::InvalidNodeId;
  SL::NodeId condition = SL::InvalidNodeId;
  SL::NodeId forStep = SL::InvalidNodeId;

  for (SL::NodeId tmp = fileContent->Sibling(forNode); tmp;
       tmp = fileContent->Sibling(tmp)) {
    SL::VObjectType const kType = fileContent->Type(tmp);
    if (kType == SL::VObjectType::paFor_initialization && !forInit) {
      forInit = tmp;
    } else if (kType == SL::VObjectType::paExpression && !condition) {
      condition = tmp;
    } else if (kType == SL::VObjectType::paFor_step && !forStep) {
      forStep = tmp;
    }
  }

  if (forInit) {
    std::string_view const kName = ExtractName(fileContent, forInit, "");
    if (!kName.empty()) {
      return kName;
    }
  }
  if (condition) {
    std::string_view const kName = ExtractName(fileContent, condition, "");
    if (!kName.empty()) {
      return kName;
    }
  }
  if (forStep) {
    std::string_view const kName = ExtractName(fileContent, forStep, "");
    if (!kName.empty()) {
      return kName;
    }
  }

  return "<unknown>";
}

auto ExtractVariableName(const SL::FileContent* fileContent,
                         SL::NodeId parentNode) -> std::string_view {
  if (fileContent == nullptr || !parentNode) {
    return "<unknown>";
  }

  auto listNodes = fileContent->sl_collect_all(
      parentNode, SL::VObjectType::paList_of_variable_decl_assignments);
  for (SL::NodeId const kListNode : listNodes) {
    auto assignNodes = fileContent->sl_collect_all(
        kListNode, SL::VObjectType::paVariable_decl_assignment);
    for (SL::NodeId const kAssignNode : assignNodes) {
      SL::NodeId const kNameNode = fileContent->Child(kAssignNode);
      if (kNameNode &&
          fileContent->Type(kNameNode) == SL::VObjectType::slStringConst) {
        return fileContent->SymName(kNameNode);
      }
    }
  }
  return "<unknown>";
}

auto ExtractParameterName(const SL::FileContent* fileContent,
                          SL::NodeId parentNode) -> std::string_view {
  if (fileContent == nullptr || !parentNode) {
    return "<unknown>";
  }

  auto listNodes = fileContent->sl_collect_all(
      parentNode, SL::VObjectType::paList_of_param_assignments);
  for (SL::NodeId const kListNode : listNodes) {
    auto assignNodes = fileContent->sl_collect_all(
        kListNode, SL::VObjectType::paParam_assignment);
    for (SL::NodeId const kAssignNode : assignNodes) {
      SL::NodeId const kNameNode = fileContent->Child(kAssignNode);
      if (kNameNode &&
          fileContent->Type(kNameNode) == SL::VObjectType::slStringConst) {
        return fileContent->SymName(kNameNode);
      }
    }
  }
  return "<unknown>";
}

namespace {
auto LvalueHasIndex(const SL::FileContent* fileContent, SL::NodeId lvalueNode) {
  for (SL::NodeId ch = fileContent->Child(lvalueNode); ch;
       ch = fileContent->Sibling(ch)) {
    SL::VObjectType const kCont = fileContent->Type(ch);

    if (kCont == SL::VObjectType::paSelect) {
      SL::NodeId const kBitSel = fileContent->Child(ch);
      if (kBitSel &&
          fileContent->Type(kBitSel) == SL::VObjectType::paBit_select) {
        if (fileContent->Child(kBitSel)) {
          return true;
        }
      }
    }

    if (kCont == SL::VObjectType::paConstant_select) {
      SL::NodeId const kBitSel = fileContent->Child(ch);
      if (kBitSel && fileContent->Type(kBitSel) ==
                         SL::VObjectType::paConstant_bit_select) {
        if (fileContent->Child(kBitSel)) {
          return true;
        }
      }
    }
  }
  return false;
}

auto IsDirectAssignment(SL::VObjectType type) -> bool {
  return type == SL::VObjectType::paOperator_assignment ||
         type == SL::VObjectType::paBlocking_assignment ||
         type == SL::VObjectType::paNonblocking_assignment ||
         type == SL::VObjectType::paNet_assignment;
}

auto IsTransparentWrapper(SL::VObjectType type) -> bool {
  return type == SL::VObjectType::paAssignment_pattern_expression ||
         type == SL::VObjectType::paConstant_assignment_pattern_expression ||
         type == SL::VObjectType::paPrimary ||
         type == SL::VObjectType::paConstant_primary ||
         type == SL::VObjectType::paConstant_expression ||
         type == SL::VObjectType::paConstant_mintypmax_expression ||
         type == SL::VObjectType::paConstant_param_expression;
}

auto LvalueName(const SL::FileContent* fileContent, SL::NodeId assignNode)
    -> std::string_view {
  for (SL::NodeId child = fileContent->Child(assignNode); child;
       child = fileContent->Sibling(child)) {
    SL::VObjectType const kCont = fileContent->Type(child);
    if (kCont == SL::VObjectType::paVariable_lvalue ||
        kCont == SL::VObjectType::paNet_lvalue) {
      if (LvalueHasIndex(fileContent, child)) {
        return "<indexed>";
      }
      return ExtractName(fileContent, child);
    }
  }
  return "<unknown>";
}

auto DeclAssignmentName(const SL::FileContent* fileContent, SL::NodeId node)
    -> std::string_view {
  SL::NodeId const kNameNode = fileContent->Child(node);
  if (kNameNode &&
      fileContent->Type(kNameNode) == SL::VObjectType::slStringConst) {
    return fileContent->SymName(kNameNode);
  }
  return "<unknown>";
}

auto ExpressionIsCompound(const SL::FileContent* fileContent, SL::NodeId node)
    -> bool {
  for (SL::NodeId ch = fileContent->Child(node); ch;
       ch = fileContent->Sibling(ch)) {
    SL::VObjectType const kCont = fileContent->Type(ch);
    if (kCont != SL::VObjectType::paExpression &&
        kCont != SL::VObjectType::paPrimary &&
        kCont != SL::VObjectType::paSelect &&
        kCont != SL::VObjectType::paBit_select &&
        kCont != SL::VObjectType::slStringConst) {
      return true;
    }
  }
  return false;
}
}  // namespace

auto FindDirectRhsLhsName(const SL::FileContent* fileContent,
                          SL::NodeId concatNode) -> std::string_view {
  SL::NodeId current = fileContent->Parent(concatNode);
  while (current) {
    SL::VObjectType const kType = fileContent->Type(current);

    if (IsDirectAssignment(kType)) {
      return LvalueName(fileContent, current);
    }

    if (kType == SL::VObjectType::paVariable_decl_assignment ||
        kType == SL::VObjectType::paNet_decl_assignment) {
      return DeclAssignmentName(fileContent, current);
    }

    if (IsTransparentWrapper(kType)) {
      current = fileContent->Parent(current);
      continue;
    }

    if (kType == SL::VObjectType::paExpression) {
      if (ExpressionIsCompound(fileContent, current)) {
        return "<unknown>";
      }
      current = fileContent->Parent(current);
      continue;
    }

    return "<unknown>";
  }
  return "<unknown>";
}

void CollectNames(const SL::FileContent* fileContent, SL::NodeId root,
                  SL::VObjectType parentType, SL::VObjectType assignType,
                  std::unordered_set<std::string_view>& out) {
  for (SL::NodeId const kDeclId :
       fileContent->sl_collect_all(root, parentType)) {
    for (SL::NodeId const kAssignId :
         fileContent->sl_collect_all(kDeclId, assignType, true)) {
      SL::NodeId const kNameNode = fileContent->Child(kAssignId);
      if (kNameNode &&
          fileContent->Type(kNameNode) == SL::VObjectType::slStringConst) {
        out.insert(fileContent->SymName(kNameNode));
      }
    }
  }
}
