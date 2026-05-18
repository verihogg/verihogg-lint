#pragma once

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/Design.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/ErrorReporting/ErrorDefinition.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <cassert>
#include <initializer_list>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace SL = SURELOG;

auto FindEnclosingModule(const SL::FileContent* fileContent, SL::NodeId node)
    -> SL::NodeId;

auto HasSiblingOfType(const SL::FileContent* fileContent, SL::NodeId start,
                      SL::VObjectType type) -> bool;

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

auto GetClassDeclByName(const SL::FileContent* fileContent,
                        std::string_view className) -> SL::NodeId;

auto GetInterfaceClassSet(SL::Design* design)
    -> std::unordered_set<std::string>;

auto GetClassSet(SL::Design* design) -> std::unordered_set<std::string>;

auto GetClassSetFromFileContent(const SL::FileContent* fileContent)
    -> std::unordered_set<std::string>;
auto GetFullNameFromScope(const SL::FileContent* fileContent, SL::NodeId node)
    -> std::string;

auto GetSuperclassString(const SL::FileContent* fileContent, SL::NodeId node)
    -> std::string;

using NodePrunePredicate = bool (*)(const SL::FileContent*, SL::NodeId,
                                    SL::VObjectType);

auto SubtreeContainsAnyType(const SL::FileContent* fileContent, SL::NodeId root,
                            std::initializer_list<SL::VObjectType> targetTypes,
                            NodePrunePredicate shouldPrune = nullptr) -> bool;

auto FindVarOrNetDecl(const SL::FileContent* fc, SL::NodeId root,
                      std::string_view name) -> SL::NodeId;

auto GetDataType(const SL::FileContent* fc, SL::NodeId declNode) -> SL::NodeId;

auto GetTypedefName(const SL::FileContent* fc, SL::NodeId declNode)
    -> std::string_view;

auto IsModuleOrInterfaceInstance(const SL::FileContent* fc, SL::NodeId root,
                                 std::string_view varName) -> bool;

auto Is1BitScalarKeyword(SL::VObjectType type) -> bool;

auto IsIntegralType(SL::VObjectType type) -> bool;

auto IsWildcardNumberType(SL::VObjectType type) -> bool;

auto IsOperatorType(SL::VObjectType type) -> bool;

auto CollectUserDefinedTypes(const SL::FileContent* fileContent,
                             SL::NodeId root)
    -> std::unordered_set<std::string_view>;

struct SvTypeInfo {
  SL::VObjectType nodeType = SL::VObjectType::sl_INVALID_;
  std::string_view name;
};

auto ExtractTypeInfoFromDataType(const SL::FileContent* fc, SL::NodeId dataType)
    -> std::optional<SvTypeInfo>;

auto SvTypeInfosMatch(const SvTypeInfo& a, const SvTypeInfo& b) -> bool;

auto ExtractArgTypesFromPortList(const SL::FileContent* fc, SL::NodeId portList)
    -> std::vector<SvTypeInfo>;
