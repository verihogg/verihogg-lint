#pragma once

#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>

void CheckBannedDisplayWriteTasks(const SURELOG::FileContent* fileContent,
                                  SURELOG::ErrorContainer* errors,
                                  SURELOG::SymbolTable* symbols);