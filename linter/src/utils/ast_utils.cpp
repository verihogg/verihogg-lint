#include "utils/ast_utils.h"

#include "Surelog/Common/FileSystem.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"

using namespace SURELOG;

NodeId findEnclosingModule(const FileContent* fC, NodeId node) {
  NodeId current = fC->Parent(node);
  while (current) {
    if (fC->Type(current) == VObjectType::paModule_declaration) return current;
    current = fC->Parent(current);
  }
  return InvalidNodeId;
}

bool hasSiblingOfType(const FileContent* fC, NodeId start, VObjectType type) {
  for (NodeId tmp = fC->Sibling(start); tmp; tmp = fC->Sibling(tmp))
    if (fC->Type(tmp) == type) return true;
  return false;
}