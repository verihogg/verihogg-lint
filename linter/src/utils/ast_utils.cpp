#include "utils/ast_utils.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <algorithm>

namespace SL = SURELOG;

auto FindEnclosingModule(const SL::FileContent* fileContent, SL::NodeId node)
    -> SL::NodeId {
  SL::NodeId current = fileContent->Parent(node);
  while (current) {
    if (fileContent->Type(current) == SL::VObjectType::paModule_declaration) {
      return current;
    }
    current = fileContent->Parent(current);
  }
  return SL::InvalidNodeId;
}

auto HasSiblingOfType(const SL::FileContent* fileContent, SL::NodeId start,
                      SL::VObjectType type) -> bool {
  for (SL::NodeId tmp = fileContent->Sibling(start); tmp;
       tmp = fileContent->Sibling(tmp)) {
    if (fileContent->Type(tmp) == type) {
      return true;
    }
  }
  return false;
}

auto HasUnpackedDimension(const SL::FileContent* fileContent,
                          SL::NodeId varDecl) -> bool {
  return std::ranges::any_of(
      fileContent->sl_collect_all(varDecl,
                                  SL::VObjectType::paVariable_decl_assignment),
      [fileContent](SL::NodeId vda) {
        SL::NodeId const kNameNode = fileContent->Child(vda);
        if (!kNameNode) {
          return false;
        }
        return HasSiblingOfType(fileContent, kNameNode,
                                SL::VObjectType::paUnpacked_dimension) ||
               HasSiblingOfType(fileContent, kNameNode,
                                SL::VObjectType::paVariable_dimension);
      });
}

auto FindAncestorOfType(const SL::FileContent* fileContent, SL::NodeId node,
                        SL::VObjectType type) -> SL::NodeId {
  SL::NodeId current = fileContent->Parent(node);
  while (current) {
    if (fileContent->Type(current) == type) {
      return current;
    }
    current = fileContent->Parent(current);
  }
  return SL::InvalidNodeId;
}

auto FindSiblingOfType(const SL::FileContent* fileContent, SL::NodeId start,
                       SL::VObjectType type) -> SL::NodeId {
  for (SL::NodeId cur = fileContent->Sibling(start); cur;
       cur = fileContent->Sibling(cur)) {
    if (fileContent->Type(cur) == type) {
      return cur;
    }
  }
  return SL::InvalidNodeId;
}

auto FindChildOfType(const SL::FileContent* fileContent, SL::NodeId node,
                     SL::VObjectType type) -> SL::NodeId {
  for (SL::NodeId cur = fileContent->Child(node); cur;
       cur = fileContent->Sibling(cur)) {
    if (fileContent->Type(cur) == type) {
      return cur;
    }
  }
  return SL::InvalidNodeId;
}