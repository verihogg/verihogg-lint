#pragma once

#include <cassert>
#include <cstdint>
#include <string>
#include <string_view>

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
std::string getStringConst(const FileContent* fC, NodeId id);

std::string getPrefix(const FileContent* fC, NodeId id);

std::string getFullName(const FileContent* fC, NodeId id);

std::unordered_map<std::string, NodeId> getClassIds(const FileContent* fC);

std::string removeFilePrefix(std::string str);

std::string getClassScope(const FileContent* fC, NodeId funcBodyId);

std::unordered_set<std::string> getInterfaceClassSet(const FileContent* fC);

std::unordered_set<std::string> getClassSet(const FileContent* fC);
