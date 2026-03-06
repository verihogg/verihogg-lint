#include "utils/name_utils.h"

#include "Surelog/Common/FileSystem.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"

using namespace SURELOG;

std::string extractName(const FileContent* fC, NodeId node,
                        const std::string& defaultName) {
  if (!fC || !node) return defaultName;

  auto stringNodes = fC->sl_collect_all(node, VObjectType::slStringConst);
  for (NodeId nameNode : stringNodes) {
    if (nameNode && fC->Type(nameNode) == VObjectType::slStringConst) {
      return std::string(fC->SymName(nameNode));
    }
  }

  NodeId child = fC->Child(node);
  if (child && fC->Type(child) == VObjectType::slStringConst) {
    return std::string(fC->SymName(child));
  }

  return defaultName;
}

std::string findForLoopVariableName(const FileContent* fC, NodeId forNode) {
  if (!fC || !forNode) return "<unknown>";

  NodeId forInit = InvalidNodeId;
  NodeId condition = InvalidNodeId;
  NodeId forStep = InvalidNodeId;

  for (NodeId tmp = fC->Sibling(forNode); tmp; tmp = fC->Sibling(tmp)) {
    VObjectType t = fC->Type(tmp);
    if (t == VObjectType::paFor_initialization && !forInit) {
      forInit = tmp;
    } else if (t == VObjectType::paExpression && !condition) {
      condition = tmp;
    } else if (t == VObjectType::paFor_step && !forStep) {
      forStep = tmp;
    }
  }

  if (forInit) {
    std::string name = extractName(fC, forInit, "");
    if (!name.empty()) return name;
  }
  if (condition) {
    std::string name = extractName(fC, condition, "");
    if (!name.empty()) return name;
  }
  if (forStep) {
    std::string name = extractName(fC, forStep, "");
  }

  return "<unknown>";
}

std::string extractVariableName(const FileContent* fC, NodeId parentNode) {
  if (!fC || !parentNode) return "<unknown>";

  auto listNodes = fC->sl_collect_all(
      parentNode, VObjectType::paList_of_variable_decl_assignments);
  for (NodeId listNode : listNodes) {
    auto assignNodes =
        fC->sl_collect_all(listNode, VObjectType::paVariable_decl_assignment);
    for (NodeId assignNode : assignNodes) {
      NodeId nameNode = fC->Child(assignNode);
      if (nameNode && fC->Type(nameNode) == VObjectType::slStringConst) {
        return std::string(fC->SymName(nameNode));
      }
    }
  }
  return "<unknown>";
}

std::string extractParameterName(const FileContent* fC, NodeId parentNode) {
  if (!fC || !parentNode) return "<unknown>";

  auto listNodes =
      fC->sl_collect_all(parentNode, VObjectType::paList_of_param_assignments);
  for (NodeId listNode : listNodes) {
    auto assignNodes =
        fC->sl_collect_all(listNode, VObjectType::paParam_assignment);
    for (NodeId assignNode : assignNodes) {
      NodeId nameNode = fC->Child(assignNode);
      if (nameNode && fC->Type(nameNode) == VObjectType::slStringConst) {
        return std::string(fC->SymName(nameNode));
      }
    }
  }
  return "<unknown>";
}

bool lvalueHasIndex(const FileContent* fC, NodeId lvalueNode) {
  for (NodeId ch = fC->Child(lvalueNode); ch; ch = fC->Sibling(ch)) {
    VObjectType ct = fC->Type(ch);

    if (ct == VObjectType::paSelect) {
      NodeId bitSel = fC->Child(ch);
      if (bitSel && fC->Type(bitSel) == VObjectType::paBit_select)
        if (fC->Child(bitSel)) return true;
    }

    if (ct == VObjectType::paConstant_select) {
      NodeId bitSel = fC->Child(ch);
      if (bitSel && fC->Type(bitSel) == VObjectType::paConstant_bit_select)
        if (fC->Child(bitSel)) return true;
    }
  }
  return false;
}

std::string findDirectRhsLhsName(const FileContent* fC, NodeId concatNode) {
  NodeId current = fC->Parent(concatNode);
  while (current) {
    VObjectType type = fC->Type(current);

    if (type == VObjectType::paOperator_assignment ||
        type == VObjectType::paBlocking_assignment ||
        type == VObjectType::paNonblocking_assignment ||
        type == VObjectType::paNet_assignment) {
      for (NodeId child = fC->Child(current); child;
           child = fC->Sibling(child)) {
        VObjectType ct = fC->Type(child);
        if (ct == VObjectType::paVariable_lvalue ||
            ct == VObjectType::paNet_lvalue) {
          if (lvalueHasIndex(fC, child)) return "<indexed>";
          return extractName(fC, child);
        }
      }
      return "<unknown>";
    }

    if (type == VObjectType::paVariable_decl_assignment) {
      NodeId nameNode = fC->Child(current);
      if (nameNode && fC->Type(nameNode) == VObjectType::slStringConst)
        return std::string(fC->SymName(nameNode));
      return "<unknown>";
    }

    if (type == VObjectType::paNet_decl_assignment) {
      NodeId nameNode = fC->Child(current);
      if (nameNode && fC->Type(nameNode) == VObjectType::slStringConst)
        return std::string(fC->SymName(nameNode));
      return "<unknown>";
    }

    if (type == VObjectType::paAssignment_pattern_expression ||
        type == VObjectType::paConstant_assignment_pattern_expression ||
        type == VObjectType::paPrimary ||
        type == VObjectType::paConstant_primary ||
        type == VObjectType::paConstant_expression ||
        type == VObjectType::paConstant_mintypmax_expression ||
        type == VObjectType::paConstant_param_expression) {
      current = fC->Parent(current);
      continue;
    }

    if (type == VObjectType::paPrimary || type == VObjectType::paExpression) {
      if (type == VObjectType::paExpression) {
        for (NodeId ch = fC->Child(current); ch; ch = fC->Sibling(ch)) {
          VObjectType ct = fC->Type(ch);
          if (ct != VObjectType::paExpression && ct != VObjectType::paPrimary &&
              ct != VObjectType::paSelect && ct != VObjectType::paBit_select &&
              ct != VObjectType::slStringConst)
            return "<unknown>";
        }
      }
      current = fC->Parent(current);
      continue;
    }

    return "<unknown>";
  }
  return "<unknown>";
}
