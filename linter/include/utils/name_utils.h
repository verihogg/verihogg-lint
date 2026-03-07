#pragma once

#include <cstdint>
#include <string_view>

#include "Surelog/Design/FileContent.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"
#include "Surelog/SourceCompile/VObjectTypes.h"

using namespace SURELOG;

//Извлечь первое имя StringConst из поддерева узла.
//Возвращает defaultName, если имя не найдено.
std::string_view extractName(const FileContent* fC, NodeId node,
                             const std::string_view& defaultName = "<unknown>");

//Извлечь параметр цикла
//Используется для определения порпуска параметра
std::string_view findForLoopVariableName(const FileContent* fC, NodeId forNode);

//Извлечь имя переменной из шаблона Variable_decl_assignment.
//Используется для поиска имен переменных в объявлениях.
std::string_view extractVariableName(const FileContent* fC, NodeId parentNode);

//Извлечение имени параметра из шаблона Param_assignment.
//Используется для поиска имен параметров в декларациях.
std::string_view extractParameterName(const FileContent* fC, NodeId parentNode);

std::string_view findDirectRhsLhsName(const FileContent* fC, NodeId concatNode);

void collectNames(const FileContent* fC, NodeId root, VObjectType parentType,
                  VObjectType assignType,
                  std::unordered_set<std::string_view>& out);
