#include "rules/missing_task_implementation.h"

#include "rules/missing_method_implementation_common.h"

void CheckMissingTaskImplementation(SURELOG::Design* design,
                                    SURELOG::ErrorContainer* errors,
                                    SURELOG::SymbolTable* symbols) {
  CheckMissingMethodImplementation(design, errors, symbols, MethodKind::kTask);
}
