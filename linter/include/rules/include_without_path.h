#pragma once

namespace SURELOG {

class Design;
class ErrorContainer;
class SymbolTable;
}  // namespace SURELOG

void CheckIncludeWithoutPath(SURELOG::Design* design,
                             SURELOG::ErrorContainer* errors,
                             SURELOG::SymbolTable* symbols);