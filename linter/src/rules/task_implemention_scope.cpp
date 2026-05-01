#include "rules/task_implemention_scope.h"

#include "rules/method_implemention_scope_common.h"

void CheckTaskImplScope(SURELOG::Design* design,
                        SURELOG::ErrorContainer* errors,
                        SURELOG::SymbolTable* symbols) {
  CheckImplScopeForKind(design, errors, symbols, ImplKind::kTask);
}
