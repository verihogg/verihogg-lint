#pragma once

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <string_view>
#include <unordered_map>

namespace ClassScopeUtils {

struct ClassScopeInfo {
  SURELOG::VObjectType scopeType{};
  std::string_view scopeName{};
};

auto FindScopeContainer(const SURELOG::FileContent* fileContent,
                        SURELOG::NodeId node) -> SURELOG::NodeId;

auto GetScopeContainerName(const SURELOG::FileContent* fileContent,
                           SURELOG::NodeId scopeContainer) -> std::string_view;

auto BuildClassScopeMap(const SURELOG::FileContent* fileContent)
    -> std::unordered_map<std::string_view, ClassScopeInfo>;

auto ScopesMatch(const ClassScopeInfo& implScope,
                 const ClassScopeInfo& classScope) -> bool;

}  // namespace ClassScopeUtils