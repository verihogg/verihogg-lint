#pragma once

#include <Surelog/Design/Design.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>

enum class MethodKind : unsigned char {
  kFunction,
  kTask,
};

void CheckMissingMethodImplementation(SURELOG::Design* design,
                                      SURELOG::ErrorContainer* errors,
                                      SURELOG::SymbolTable* symbols,
                                      MethodKind kind);
