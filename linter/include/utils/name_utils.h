#pragma once

#include <cstdint>
#include <string>

#include "Surelog/Design/FileContent.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"
#include "Surelog/SourceCompile/VObjectTypes.h"

using namespace SURELOG;

//Извлечь первое имя StringConst из поддерева узла.
//Возвращает defaultName, если имя не найдено.
std::string extractName(const FileContent* fC, NodeId node,
                        const std::string& defaultName = "<unknown>");

//Извлечь параметр цикла
//Используется для определения порпуска параметра
std::string findForLoopVariableName(const FileContent* fC, NodeId forNode);

//Извлечь имя переменной из шаблона Variable_decl_assignment.
//Используется для поиска имен переменных в объявлениях.
std::string extractVariableName(const FileContent* fC, NodeId parentNode);

//Извлечение имени параметра из шаблона Param_assignment.
//Используется для поиска имен параметров в декларациях.
std::string extractParameterName(const FileContent* fC, NodeId parentNode);

std::string findLhsVariableName(const FileContent* fC, NodeId startNode);


