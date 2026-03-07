#pragma once

#include <cstdint>
#include <string_view>

#include "Surelog/Design/FileContent.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"
#include "Surelog/SourceCompile/VObjectTypes.h"

using namespace SURELOG;

//Безопасно извлекает номер столбца из узла, возвращая 0, если он недоступен.
uint32_t getColumnSafe(const FileContent* fC, NodeId node);

//Создать объект Location из узла.
Location getLocation(const FileContent* fC, NodeId node,
                     const std::string_view& symbolName, SymbolTable* symbols);

//Сообщить об ошибке линтинга в определенном месте узла.
//Это основная функция отчетности об ошибках, используемая всеми правилами.
void reportError(const FileContent* fC, NodeId node,
                 const std::string_view& symbolName,
                 ErrorDefinition::ErrorType errorType, ErrorContainer* errors,
                 SymbolTable* symbols);

NodeId findArrayIdNode(const FileContent* fC, NodeId foreachKeyword);