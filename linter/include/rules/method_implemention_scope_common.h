#pragma once

#include <Surelog/Design/Design.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>

enum class ImplKind : unsigned char { kFunction, kTask, kConstraint };

void CheckImplScopeForKind(SURELOG::Design* design,
                           SURELOG::ErrorContainer* errors,
                           SURELOG::SymbolTable* symbols, ImplKind kind);
