#pragma once

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/ErrorReporting/ErrorDefinition.h>
#include <Surelog/SourceCompile/SymbolTable.h>

#include <cassert>
#include <string>

namespace SL = SURELOG;

auto FindEnclosingModule(const SL::FileContent* fileContent, SL::NodeId node)
    -> SL::NodeId;

auto HasSiblingOfType(const SL::FileContent* fileContent, SL::NodeId start,
                      SL::VObjectType type) -> bool;

const SL::NodeId zeroId = SL::NodeId(SL::InvalidRawNodeId); /*  */

auto HasUnpackedDimension(const SL::FileContent* fileContent,
                          SL::NodeId varDecl) -> bool;

auto FindAncestorOfType(const SL::FileContent* fileContent, SL::NodeId node,
                        SL::VObjectType type) -> SL::NodeId;

auto FindSiblingOfType(const SL::FileContent* fileContent, SL::NodeId start,
                       SL::VObjectType type) -> SL::NodeId;

auto FindChildOfType(const SL::FileContent* fileContent, SL::NodeId node,
                     SL::VObjectType type) -> SL::NodeId;
auto GetStringConst(const SL::FileContent* fileContent, SL::NodeId node)
    -> std::string;

auto GetPrefix(const SL::FileContent* fileContent, SL::NodeId node)
    -> std::string;

auto IsBuiltinClass(const std::string& className) -> bool;

auto GetFullName(const SL::FileContent* fileContent, SL::NodeId node)
    -> std::string;

auto GetClassIds(const SL::FileContent* fileContent)
    -> std::unordered_map<std::string, SL::NodeId>;

auto RemoveFilePrefix(std::string str) -> std::string;

auto GetClassScope(const SL::FileContent* fileContent, SL::NodeId funcBodyNode)
    -> std::vector<std::string>;

auto GetInterfaceClassSet(const SL::FileContent* fileContent)
    -> std::unordered_set<std::string>;

auto GetClassSet(const SL::FileContent* fileContent)
    -> std::unordered_set<std::string>;

auto GetFullNameFromScope(const SL::FileContent* fileContent, SL::NodeId node)
    -> std::string;

auto GetSuperclassString(const SL::FileContent* fileContent, SL::NodeId node)
    -> std::string;
