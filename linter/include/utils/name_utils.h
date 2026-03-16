#pragma once

#include <cstdint>
#include <string_view>

#include "Surelog/Design/FileContent.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"
#include "Surelog/SourceCompile/VObjectTypes.h"

//Извлечь первое имя StringConst из поддерева узла.
//Возвращает defaultName, если имя не найдено.
auto ExtractName(const SURELOG::FileContent* fileContent, SURELOG::NodeId node,
                 const std::string_view& defaultName = "<unknown>")
    -> std::string_view;

//Извлечь параметр цикла
//Используется для определения порпуска параметра
auto FindForLoopVariableName(const SURELOG::FileContent* fileContent,
                             SURELOG::NodeId forNode) -> std::string_view;

//Извлечь имя переменной из шаблона Variable_decl_assignment.
//Используется для поиска имен переменных в объявлениях.
auto ExtractVariableName(const SURELOG::FileContent* fileContent,
                         SURELOG::NodeId parentNode) -> std::string_view;

//Извлечение имени параметра из шаблона Param_assignment.
//Используется для поиска имен параметров в декларациях.
auto ExtractParameterName(const SURELOG::FileContent* fileContent,
                          SURELOG::NodeId parentNode) -> std::string_view;

auto FindDirectRhsLhsName(const SURELOG::FileContent* fileContent,
                          SURELOG::NodeId concatNode) -> std::string_view;

void CollectNames(const SURELOG::FileContent* fileContent, SURELOG::NodeId root,
                  SURELOG::VObjectType parentType,
                  SURELOG::VObjectType assignType,
                  std::unordered_set<std::string_view>& out);
