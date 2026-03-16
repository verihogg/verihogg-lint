#include "utils/ast_utils.h"

#include <Surelog/Common/FileSystem.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>

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
        SL::NodeId nameNode = fileContent->Child(vda);
        if (!nameNode) {
          return false;
        }
        return HasSiblingOfType(fileContent, nameNode,
                                SL::VObjectType::paUnpacked_dimension) ||
               HasSiblingOfType(fileContent, nameNode,
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