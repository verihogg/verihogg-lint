#include "utils/name_utils.h"

#include <Surelog/Common/FileSystem.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>

namespace SL = SURELOG;

auto ExtractName(const SL::FileContent* fileContent, SL::NodeId node,
                 const std::string_view& defaultName) -> std::string_view {
  if (fileContent == nullptr || !node) {
    return defaultName;
  }

  auto stringNodes =
      fileContent->sl_collect_all(node, SL::VObjectType::slStringConst);
  for (SL::NodeId nameNode : stringNodes) {
    if (nameNode &&
        fileContent->Type(nameNode) == SL::VObjectType::slStringConst) {
      return fileContent->SymName(nameNode);
    }
  }

  SL::NodeId child = fileContent->Child(node);
  if (child && fileContent->Type(child) == SL::VObjectType::slStringConst) {
    return fileContent->SymName(child);
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
    SL::VObjectType type = fileContent->Type(tmp);
    if (type == SL::VObjectType::paFor_initialization && !forInit) {
      forInit = tmp;
    } else if (type == SL::VObjectType::paExpression && !condition) {
      condition = tmp;
    } else if (type == SL::VObjectType::paFor_step && !forStep) {
      forStep = tmp;
    }
  }

  if (forInit) {
    std::string_view name = ExtractName(fileContent, forInit, "");
    if (!name.empty()) {
      return name;
    }
  }
  if (condition) {
    std::string_view name = ExtractName(fileContent, condition, "");
    if (!name.empty()) {
      return name;
    }
  }
  if (forStep) {
    std::string_view name = ExtractName(fileContent, forStep, "");
    if (!name.empty()) {
      return name;
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
  for (SL::NodeId listNode : listNodes) {
    auto assignNodes = fileContent->sl_collect_all(
        listNode, SL::VObjectType::paVariable_decl_assignment);
    for (SL::NodeId assignNode : assignNodes) {
      SL::NodeId nameNode = fileContent->Child(assignNode);
      if (nameNode &&
          fileContent->Type(nameNode) == SL::VObjectType::slStringConst) {
        return fileContent->SymName(nameNode);
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
  for (SL::NodeId listNode : listNodes) {
    auto assignNodes = fileContent->sl_collect_all(
        listNode, SL::VObjectType::paParam_assignment);
    for (SL::NodeId assignNode : assignNodes) {
      SL::NodeId nameNode = fileContent->Child(assignNode);
      if (nameNode &&
          fileContent->Type(nameNode) == SL::VObjectType::slStringConst) {
        return fileContent->SymName(nameNode);
      }
    }
  }
  return "<unknown>";
}

static auto LvalueHasIndex(const SL::FileContent* fileContent,
                           SL::NodeId lvalueNode) {
  for (SL::NodeId ch = fileContent->Child(lvalueNode); ch;
       ch = fileContent->Sibling(ch)) {
    SL::VObjectType cont = fileContent->Type(ch);

    if (cont == SL::VObjectType::paSelect) {
      SL::NodeId bitSel = fileContent->Child(ch);
      if (bitSel &&
          fileContent->Type(bitSel) == SL::VObjectType::paBit_select) {
        if (fileContent->Child(bitSel)) {
          return true;
        }
      }
    }

    if (cont == SL::VObjectType::paConstant_select) {
      SL::NodeId bitSel = fileContent->Child(ch);
      if (bitSel &&
          fileContent->Type(bitSel) == SL::VObjectType::paConstant_bit_select) {
        if (fileContent->Child(bitSel)) {
          return true;
        }
      }
    }
  }
  return false;
}

static auto IsDirectAssignment(SL::VObjectType type) -> bool {
  return type == SL::VObjectType::paOperator_assignment ||
         type == SL::VObjectType::paBlocking_assignment ||
         type == SL::VObjectType::paNonblocking_assignment ||
         type == SL::VObjectType::paNet_assignment;
}

static auto IsTransparentWrapper(SL::VObjectType type) -> bool {
  return type == SL::VObjectType::paAssignment_pattern_expression ||
         type == SL::VObjectType::paConstant_assignment_pattern_expression ||
         type == SL::VObjectType::paPrimary ||
         type == SL::VObjectType::paConstant_primary ||
         type == SL::VObjectType::paConstant_expression ||
         type == SL::VObjectType::paConstant_mintypmax_expression ||
         type == SL::VObjectType::paConstant_param_expression;
}

static auto LvalueName(const SL::FileContent* fileContent,
                       SL::NodeId assignNode) -> std::string_view {
  for (SL::NodeId child = fileContent->Child(assignNode); child;
       child = fileContent->Sibling(child)) {
    SL::VObjectType cont = fileContent->Type(child);
    if (cont == SL::VObjectType::paVariable_lvalue ||
        cont == SL::VObjectType::paNet_lvalue) {
      if (LvalueHasIndex(fileContent, child)) {
        return "<indexed>";
      }
      return ExtractName(fileContent, child);
    }
  }
  return "<unknown>";
}

static auto DeclAssignmentName(const SL::FileContent* fileContent,
                               SL::NodeId node) -> std::string_view {
  SL::NodeId nameNode = fileContent->Child(node);
  if (nameNode &&
      fileContent->Type(nameNode) == SL::VObjectType::slStringConst) {
    return fileContent->SymName(nameNode);
  }
  return "<unknown>";
}

static auto ExpressionIsCompound(const SL::FileContent* fileContent,
                                 SL::NodeId node) -> bool {
  for (SL::NodeId ch = fileContent->Child(node); ch;
       ch = fileContent->Sibling(ch)) {
    SL::VObjectType cont = fileContent->Type(ch);
    if (cont != SL::VObjectType::paExpression &&
        cont != SL::VObjectType::paPrimary &&
        cont != SL::VObjectType::paSelect &&
        cont != SL::VObjectType::paBit_select &&
        cont != SL::VObjectType::slStringConst) {
      return true;
    }
  }
  return false;
}

auto FindDirectRhsLhsName(const SL::FileContent* fileContent,
                          SL::NodeId concatNode) -> std::string_view {
  SL::NodeId current = fileContent->Parent(concatNode);
  while (current) {
    SL::VObjectType type = fileContent->Type(current);

    if (IsDirectAssignment(type)) {
      return LvalueName(fileContent, current);
    }

    if (type == SL::VObjectType::paVariable_decl_assignment ||
        type == SL::VObjectType::paNet_decl_assignment) {
      return DeclAssignmentName(fileContent, current);
    }

    if (IsTransparentWrapper(type)) {
      current = fileContent->Parent(current);
      continue;
    }

    if (type == SL::VObjectType::paExpression) {
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
  for (SL::NodeId declId : fileContent->sl_collect_all(root, parentType)) {
    for (SL::NodeId assignId :
         fileContent->sl_collect_all(declId, assignType, true)) {
      SL::NodeId nameNode = fileContent->Child(assignId);
      if (nameNode &&
          fileContent->Type(nameNode) == SL::VObjectType::slStringConst) {
        out.insert(fileContent->SymName(nameNode));
      }
    }
  }
}
