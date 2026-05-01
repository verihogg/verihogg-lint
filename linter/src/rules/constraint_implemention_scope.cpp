#include "rules/constraint_implemention_scope.h"

#include "rules/method_implemention_scope_common.h"

void CheckConstraintImplScope(SURELOG::Design* design,
                              SURELOG::ErrorContainer* errors,
                              SURELOG::SymbolTable* symbols) {
  CheckImplScopeForKind(design, errors, symbols, ImplKind::kConstraint);
}