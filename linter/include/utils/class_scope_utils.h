#pragma once

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <optional>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace ClassScopeUtils {

struct ClassScopeInfo {
  SURELOG::VObjectType scopeType{SURELOG::VObjectType::sl_INVALID_};
  std::string_view scopeName;
};

struct ClassDeclInfo {
  std::string_view className;
  ClassScopeInfo scopeInfo;
};

struct ClassScopedMemberInfo {
  std::string_view className;
  std::string_view memberName;
  SURELOG::NodeId classScopeNode;
};

auto FindScopeContainer(const SURELOG::FileContent* fileContent,
                        SURELOG::NodeId node) -> SURELOG::NodeId;

auto GetScopeContainerName(const SURELOG::FileContent* fileContent,
                           SURELOG::NodeId scopeContainer) -> std::string_view;

auto BuildClassScopeMap(const SURELOG::FileContent* fileContent)
    -> std::unordered_map<std::string_view, ClassScopeInfo>;

auto CollectClassDeclInfos(const SURELOG::FileContent* fileContent)
    -> std::vector<ClassDeclInfo>;

auto ExtractClassScopedMember(const SURELOG::FileContent* fileContent,
                              SURELOG::NodeId node)
    -> std::optional<ClassScopedMemberInfo>;

auto ScopesMatch(const ClassScopeInfo& implScope,
                 const ClassScopeInfo& classScope) -> bool;

}  // namespace ClassScopeUtils
