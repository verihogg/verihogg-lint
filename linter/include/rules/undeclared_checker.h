#pragma once

namespace SURELOG {
class Design;
class ErrorContainer;
class SymbolTable;
}  // namespace SURELOG

void CheckUndeclaredChecker(SURELOG::Design* design,
                            SURELOG::ErrorContainer* errors,
                            SURELOG::SymbolTable* symbols);
