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
      [fileContent](SL::NodeId vda) -> bool {
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

auto isBuiltinClass(const std::string& className) -> bool {
  return (className == "semaphore" || className == "process" ||
          className == "mailbox");
}

auto getStringConst(const SL::FileContent* fC, SL::NodeId id) -> std::string {
  const SL::NodeId kString = fC->sl_get(id, SL::VObjectType::slStringConst);
  std::string str{fC->SymName(kString)};
  return str;
}

auto getPrefix(const SL::FileContent* fC, SL::NodeId id) -> std::string {
  std::vector<std::string> contexts;
  std::string libName{fC->getLibrary()->getName()};
  SL::NodeId tempId = fC->Parent(id);

  while (tempId) {
    SL::NodeId intermediateTempId;
    SL::VObjectType type = fC->Type(tempId);

    switch (type) {
      case SL::VObjectType::paProgram_declaration: {
        intermediateTempId =
            fC->sl_get(tempId, SL::VObjectType::paProgram_ansi_header);
        break;
      }
      case SL::VObjectType::paInterface_declaration: {
        intermediateTempId =
            fC->sl_get(tempId, SL::VObjectType::paInterface_ansi_header);
        intermediateTempId = fC->sl_get(
            intermediateTempId, SL::VObjectType::paInterface_identifier);
        break;
      }
      case SL::VObjectType::paModule_declaration: {
        intermediateTempId =
            fC->sl_get(tempId, SL::VObjectType::paModule_ansi_header);
        break;
      }
      case SL::VObjectType::paClass_declaration:
      case SL::VObjectType::paPackage_declaration: {
        intermediateTempId = fC->sl_get(tempId, SL::VObjectType::slStringConst);
        break;
      }
      default: {
        tempId = fC->Parent(tempId);
        continue;
      }
    }

    std::string stringConst = getStringConst(fC, intermediateTempId);
    contexts.push_back(stringConst);
    tempId = fC->Parent(tempId);
  }

  std::string result;
  for (size_t i = 0; i < contexts.size(); i++) {
    if (i > 0) {
      result += "::";
    }
    result += contexts[i];
  }

  return libName + "@" + result;
}

auto getFullName(const SL::FileContent* fC, SL::NodeId id) -> std::string {
  const std::string kName = getStringConst(fC, id);
  std::string kPrefix = getPrefix(fC, id);
  if (kName == "") {
    return kPrefix;
  }
  return kPrefix + kName;
}
