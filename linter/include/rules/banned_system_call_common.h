#pragma once

#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>

#include <span>
#include <string_view>

#include "main/lint_rules.h"

// Names are given without the leading '$', e.g. "finish".
void CheckBannedSystemCallsImpl(const SURELOG::FileContent* fileContent,
                                SURELOG::ErrorContainer* errors,
                                SURELOG::SymbolTable* symbols,
                                std::span<const std::string_view> bannedNames,
                                verihogg_lint::LintId ruleId);
