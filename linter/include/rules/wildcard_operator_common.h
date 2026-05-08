#pragma once

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/ErrorReporting/ErrorDefinition.h>
#include <Surelog/SourceCompile/SymbolTable.h>

#include <cstdint>
#include <string_view>
#include <vector>

#include "fix/source_manager.h"
#include "main/lint_diagnostics.h"
#include "main/lint_rules.h"

enum class WildcardOperatorKind : std::uint8_t {
  kUnknown = 0,
  kEquality,
  kInequality,
};

auto GetWildcardLhsName(const SURELOG::FileContent* fileContent,
                        SURELOG::NodeId opNode) -> std::string_view;

auto DetectWildcardOperatorKind(const SURELOG::FileContent* fileContent,
                                SURELOG::NodeId opNode) -> WildcardOperatorKind;

struct WildcardOpStrings {
  std::string_view message;
  std::string_view correctOp;
};

auto CheckWildcardOperatorFixableImpl(const SURELOG::FileContent* fileContent,
                                      SURELOG::ErrorContainer* errors,
                                      SURELOG::SymbolTable* symbols,
                                      WildcardOperatorKind targetKind,
                                      verihogg_lint::LintId ruleId,
                                      WildcardOpStrings strings)
    -> std::vector<LintDiagnostic>;
