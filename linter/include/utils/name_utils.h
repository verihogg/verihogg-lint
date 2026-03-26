#pragma once

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <string_view>
#include <unordered_set>

namespace SL = SURELOG;
// Извлечь первое имя StringConst из поддерева узла.
// Возвращает defaultName, если имя не найдено.
auto ExtractName(const SL::FileContent* fileContent, SL::NodeId node,
                 const std::string_view& defaultName = "<unknown>")
    -> std::string_view;

// Извлечь параметр цикла
// Используется для определения порпуска параметра
auto FindForLoopVariableName(const SL::FileContent* fileContent,
                             SL::NodeId forNode) -> std::string_view;

// Извлечь имя переменной из шаблона Variable_decl_assignment.
// Используется для поиска имен переменных в объявлениях.
auto ExtractVariableName(const SL::FileContent* fileContent,
                         SL::NodeId parentNode) -> std::string_view;

// Извлечение имени параметра из шаблона Param_assignment.
// Используется для поиска имен параметров в декларациях.
auto ExtractParameterName(const SL::FileContent* fileContent,
                          SL::NodeId parentNode) -> std::string_view;

auto FindDirectRhsLhsName(const SL::FileContent* fileContent,
                          SL::NodeId concatNode) -> std::string_view;
void CollectNames(const SL::FileContent* fileContent, SL::NodeId root,
                  SL::VObjectType parentType, SL::VObjectType assignType,
                  std::unordered_set<std::string_view>& out);
