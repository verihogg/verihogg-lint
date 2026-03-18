#pragma once

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/ErrorReporting/ErrorDefinition.h>
#include <Surelog/SourceCompile/SymbolTable.h>

#include <string_view>

namespace SL = SURELOG;
// Безопасно извлекает номер столбца из узла, возвращая 0, если он недоступен.
auto GetColumnSafe(const SL::FileContent* fileContent, SL::NodeId node);

// Создать объект Location из узла.
auto GetLocation(const SL::FileContent* fileContent, SL::NodeId node,
                 const std::string_view& symbolName, SL::SymbolTable* symbols);

// Сообщить об ошибке линтинга в определенном месте узла.
// Это основная функция отчетности об ошибках, используемая всеми правилами.
void ReportError(const SL::FileContent* fileContent, SL::NodeId node,
                 const std::string_view& symbolName,
                 SL::ErrorDefinition::ErrorType errorType,
                 SL::ErrorContainer* errors, SL::SymbolTable* symbols);

auto FindArrayIdNode(const SL::FileContent* fileContent,
                     SL::NodeId foreachKeyword) -> SL::NodeId;