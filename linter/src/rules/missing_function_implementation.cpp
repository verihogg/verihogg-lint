#include "rules/missing_function_implementation.h"

#include "rules/missing_method_implementation_common.h"

void CheckMissingFunctionImplementation(SURELOG::Design* design,
                                        SURELOG::ErrorContainer* errors,
                                        SURELOG::SymbolTable* symbols) {
  CheckMissingMethodImplementation(design, errors, symbols,
                                   MethodKind::kFunction);
}
