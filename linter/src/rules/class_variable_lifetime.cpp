#include "rules/class_variable_lifetime.h"

#include <Surelog/Common/FileSystem.h>
#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <iostream>
#include <limits>
#include <string>
#include <string_view>
#include <vector>

#include "fix/fix_it.h"
#include "main/lint_diagnostics.h"
#include "main/lint_rules.h"
#include "utils/location_utils.h"
#include "utils/name_utils.h"

namespace SL = SURELOG;

auto CheckClassVariableLifetimeFixable(const SL::FileContent* fC,
                                       SL::ErrorContainer* errors,
                                       SL::SymbolTable* symbols,
                                       FixSourceManager& sm)
    -> std::vector<LintDiagnostic> {
  std::vector<LintDiagnostic> diags;

  if (fC == nullptr || errors == nullptr || symbols == nullptr) {
    return diags;
  }

  const SL::NodeId kRoot = fC->getRootNode();
  if (!kRoot) {
    return diags;
  }

  const std::string filepath =
      std::string(SL::FileSystem::getInstance()->toPath(fC->getFileId()));

  if (filepath.empty()) {
    return diags;
  }

  for (SL::NodeId const kClassId :
       fC->sl_collect_all(kRoot, SL::VObjectType::paClass_declaration)) {
    for (SL::NodeId const kPropId :
         fC->sl_collect_all(kClassId, SL::VObjectType::paClass_property)) {
      for (SL::NodeId const kAutoId :
           fC->sl_collect_all(kPropId, SL::VObjectType::paLifetime_Automatic)) {
        const std::string_view kVarName = ExtractVariableName(fC, kPropId);

        ReportError(fC, kAutoId, kVarName,
                    verihogg_lint::LINT_CLASS_VARIABLE_LIFETIME, errors,
                    symbols);

        const unsigned line = fC->Line(kAutoId);
        const unsigned col = GetColumnSafe(fC, kAutoId);

        if (line == 0 || col == 0) {
          continue;
        }

        const unsigned token_start_0 = col - 1U;

        const std::string slice =
            sm.getSlice(filepath, LineCol{.line = line, .col = token_start_0},
                        std::numeric_limits<unsigned>::max());

        constexpr unsigned kKeywordLen = 9;

        if (slice.size() < kKeywordLen || !slice.starts_with("automatic")) {
          if (!slice.empty()) {
            std::cerr << "autofix: warning: 'automatic' not found at "
                      << filepath << ":" << line << ":" << col
                      << "; fix skipped, error remains in report\n";
          }
          continue;
        }

        unsigned remove_len = kKeywordLen;
        while (remove_len < slice.size() &&
               (slice.at(remove_len) == ' ' || slice.at(remove_len) == '\t')) {
          ++remove_len;
        }

        const FixRange remove_range{
            .begin =
                FixLocation{.filename = filepath, .line = line, .col = col},
            .end = FixLocation{
                .filename = filepath, .line = line, .col = col + remove_len}};

        LintDiagnostic d;
        d.filepath = filepath;
        d.line = line;
        d.col = col;
        d.message = "Class variable '" + std::string(kVarName) +
                    "' cannot use automatic lifetime";

        try {
          d.fixes.push_back(
              FixIt::Remove(remove_range, "remove 'automatic' keyword"));
        } catch (const std::exception& e) {
          std::cerr << "autofix: cannot create fix at " << filepath << ":"
                    << line << ":" << col << ": " << e.what() << "\n";
          continue;
        }

        diags.push_back(std::move(d));
      }
    }
  }

  return diags;
}
