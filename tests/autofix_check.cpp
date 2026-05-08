#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "Surelog/API/Surelog.h"
#include "Surelog/CommandLine/CommandLineParser.h"
#include "Surelog/Design/Design.h"
#include "Surelog/Design/FileContent.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"
#include "fix/replacement.h"
#include "fix/source_manager.h"
#include "main/lint_diagnostics.h"
#include "main/lint_rules.h"
#include "rules/assignment_pattern.h"
#include "rules/class_variable_lifetime.h"
#include "rules/exponent_format_time_value.h"
#include "rules/method_override_argument_name.h"
#include "rules/multiple_dot_star_connection.h"
#include "rules/time_value.h"
#include "rules/type_casting.h"
#include "rules/void_cast_of_void_function.h"
#include "rules/wildcard_equality_operator.h"
#include "rules/wildcard_inequality_operator.h"
#include "utils/init.h"

namespace SL = SURELOG;
namespace fs = std::filesystem;

#ifndef VERIHOGG_TEST_NON_STANDARD_ROOT
#error "VERIHOGG_TEST_NON_STANDARD_ROOT must be defined by CMake"
#endif

namespace {

using FixableCheck = std::function<std::vector<LintDiagnostic>(
    const SL::FileContent*, SL::ErrorContainer*, SL::SymbolTable*,
    FixSourceManager&)>;

using FixableGlobalCheck = std::function<std::vector<LintDiagnostic>(
    SL::Design*, SL::ErrorContainer*, SL::SymbolTable*, FixSourceManager&)>;

struct FixableRuleSpec {
  std::string folder;
  FixableCheck check;
};

struct FixableGlobalRuleSpec {
  std::string folder;
  FixableGlobalCheck check;
};

auto BasePath() -> const fs::path& {
  static const fs::path base{VERIHOGG_TEST_NON_STANDARD_ROOT};
  return base;
}

auto ReadFileText(const fs::path& path) -> std::string {
  std::ifstream f(path, std::ios::binary);
  if (!f.is_open()) {
    throw std::runtime_error("Cannot open: " + path.string());
  }
  return {std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>()};
}

auto CollectFixPairs(const fs::path& dir)
    -> std::vector<std::pair<fs::path, fs::path>> {
  std::vector<std::pair<fs::path, fs::path>> pairs;
  if (!fs::exists(dir)) {
    return pairs;
  }

  for (const auto& entry : fs::directory_iterator(dir)) {
    const fs::path& p = entry.path();
    if (!entry.is_regular_file() || p.extension() != ".sv") {
      continue;
    }
    const std::string stem = p.stem().string();
    if (stem.ends_with(".fixed")) {
      continue;
    }
    if (!stem.starts_with("case")) {
      continue;
    }
    const fs::path expected = dir / (stem + ".fixed.sv");
    if (!fs::exists(expected)) {
      std::cerr << "[AUTOFIX TEST] Missing expected file: " << expected << "\n";
      continue;
    }
    pairs.emplace_back(p, expected);
  }
  std::ranges::sort(
      pairs, [](const auto& a, const auto& b) { return a.first < b.first; });
  return pairs;
}

auto MakeCompiler(const fs::path& sv_path, SL::ErrorContainer* errors,
                  SL::SymbolTable* symbols)
    -> std::unique_ptr<SL::CommandLineParser> {
  auto clp =
      std::make_unique<SL::CommandLineParser>(errors, symbols, false, false);
  clp->noPython();
  clp->setParse(true);
  clp->setCompile(true);
  clp->setElaborate(false);
  clp->setwritePpOutput(true);
  clp->setCacheAllowed(false);
  clp->setFilterInfo();
  clp->setFilterNote();
  clp->setFilterWarning();

  const std::string path_str = sv_path.string();
  std::array<const char*, 2> argv = {"verihogg-autofix-test", path_str.c_str()};
  if (!clp->parseCommandLine(static_cast<int>(argv.size()), argv.data())) {
    throw std::runtime_error("Cannot parse command line for: " + path_str);
  }
  return clp;
}

auto GetFileContent(const fs::path& path, SL::ErrorContainer* errors,
                    SL::SymbolTable* symbols) -> SL::FileContent* {
  auto clp = MakeCompiler(path, errors, symbols);
  auto* compiler = start_compiler(clp.get());
  auto* design = get_design(compiler);
  if (design == nullptr) {
    throw std::runtime_error("Design is null for: " + path.string());
  }
  auto& contents = design->getAllFileContents();
  if (contents.empty()) {
    throw std::runtime_error("No FileContent for: " + path.string());
  }
  auto it = contents.begin();
  if (it == contents.end() || it->second == nullptr) {
    throw std::runtime_error("Null FileContent for: " + path.string());
  }
  return it->second;
}

auto GetDesign(const fs::path& path, SL::ErrorContainer* errors,
               SL::SymbolTable* symbols) -> SL::Design* {
  auto clp = MakeCompiler(path, errors, symbols);
  auto* compiler = start_compiler(clp.get());
  auto* design = get_design(compiler);
  if (design == nullptr) {
    throw std::runtime_error("Design is null for: " + path.string());
  }
  return design;
}

auto ApplyDiagFixes(const std::vector<LintDiagnostic>& diags,
                    FixSourceManager& sm, const std::string& source_text)
    -> std::string {
  Replacements repls;
  for (const auto& d : diags) {
    for (const auto& fix : d.fixes) {
      try {
        Replacement r = fixItToReplacement(fix, sm);
        r.rule_id = d.rule_id;
        std::string conflict;
        if (!repls.add(r, &conflict)) {
          std::cerr << "[AUTOFIX TEST] conflict skipped: " << conflict << "\n";
        }
      } catch (const std::exception& e) {
        std::cerr << "[AUTOFIX TEST] fixItToReplacement failed: " << e.what()
                  << "\n";
      }
    }
  }
  return repls.apply(source_text);
}

void RunFixableRuleTest(const FixableRuleSpec& spec) {
  const fs::path fix_dir = BasePath() / spec.folder / "Fix";
  const auto pairs = CollectFixPairs(fix_dir);

  if (pairs.empty()) {
    GTEST_SKIP() << "No fix cases for: " << spec.folder;
  }

  SL::ErrorDefinition::init();
  verihogg_lint::RegisterLintRules();

  for (const auto& [input_path, expected_path] : pairs) {
    SCOPED_TRACE(input_path.string());

    auto symbols = std::make_unique<SL::SymbolTable>();
    auto errors = std::make_unique<SL::ErrorContainer>(symbols.get());

    const SL::FileContent* fC = nullptr;
    try {
      fC = GetFileContent(input_path, errors.get(), symbols.get());
    } catch (const std::exception& e) {
      FAIL() << "Surelog failed on " << input_path << ": " << e.what();
    }
    ASSERT_NE(fC, nullptr);

    FixSourceManager sm;
    ASSERT_TRUE(sm.loadFile(fC->getFileId()))
        << "Cannot load file into FixSourceManager: " << input_path;

    std::vector<LintDiagnostic> diags;
    try {
      diags = spec.check(fC, errors.get(), symbols.get(), sm);
    } catch (const std::exception& e) {
      FAIL() << "Rule check threw: " << e.what();
    }

    ASSERT_FALSE(diags.empty())
        << "Rule produced no diagnostics for: " << input_path;

    const bool has_fixes = std::ranges::any_of(
        diags, [](const LintDiagnostic& d) { return !d.fixes.empty(); });
    ASSERT_TRUE(has_fixes) << "No fixes generated for: " << input_path;

    const std::string source_text = ReadFileText(input_path);
    const std::string fixed_text = ApplyDiagFixes(diags, sm, source_text);
    const std::string expected_text = ReadFileText(expected_path);

    EXPECT_EQ(fixed_text, expected_text)
        << "Fixed output does not match expected for: " << input_path;
  }
}

void RunFixableGlobalRuleTest(const FixableGlobalRuleSpec& spec) {
  const fs::path fix_dir = BasePath() / spec.folder / "Fix";
  const auto pairs = CollectFixPairs(fix_dir);

  if (pairs.empty()) {
    GTEST_SKIP() << "No fix cases for: " << spec.folder;
  }

  SL::ErrorDefinition::init();
  verihogg_lint::RegisterLintRules();

  for (const auto& [input_path, expected_path] : pairs) {
    SCOPED_TRACE(input_path.string());

    auto symbols = std::make_unique<SL::SymbolTable>();
    auto errors = std::make_unique<SL::ErrorContainer>(symbols.get());

    SL::Design* design = nullptr;
    try {
      design = GetDesign(input_path, errors.get(), symbols.get());
    } catch (const std::exception& e) {
      FAIL() << "Surelog failed on " << input_path << ": " << e.what();
    }
    ASSERT_NE(design, nullptr);

    FixSourceManager sm;
    for (auto& [name, fC] : design->getAllFileContents()) {
      if (fC != nullptr) {
        (void)sm.loadFile(fC->getFileId());
      }
    }

    std::vector<LintDiagnostic> diags;
    try {
      diags = spec.check(design, errors.get(), symbols.get(), sm);
    } catch (const std::exception& e) {
      FAIL() << "Global rule check threw: " << e.what();
    }

    ASSERT_FALSE(diags.empty())
        << "Global rule produced no diagnostics for: " << input_path;

    const bool has_fixes = std::ranges::any_of(
        diags, [](const LintDiagnostic& d) { return !d.fixes.empty(); });
    ASSERT_TRUE(has_fixes) << "No fixes generated for: " << input_path;

    std::map<std::string, Replacements> by_file;
    for (const auto& d : diags) {
      for (const auto& fix : d.fixes) {
        try {
          Replacement r = fixItToReplacement(fix, sm);
          r.rule_id = d.rule_id;
          std::string conflict;
          (void)by_file[r.filename].add(r, &conflict);
        } catch (const std::exception& e) {
          std::cerr << "[AUTOFIX TEST] fixItToReplacement failed: " << e.what()
                    << "\n";
        }
      }
    }

    const std::string input_str = fs::canonical(input_path).string();
    Replacements* repls = nullptr;
    for (auto& [fname, r] : by_file) {
      if (fs::exists(fname) && fs::canonical(fname).string() == input_str) {
        repls = &r;
        break;
      }
    }

    ASSERT_NE(repls, nullptr)
        << "No fixes found for input file: " << input_path;

    const std::string source_text = ReadFileText(input_path);
    const std::string fixed_text = repls->apply(source_text);
    const std::string expected_text = ReadFileText(expected_path);

    EXPECT_EQ(fixed_text, expected_text)
        << "Fixed output does not match expected for: " << input_path;
  }
}

auto FixableSpecs() -> const std::vector<FixableRuleSpec>& {
  static const std::vector<FixableRuleSpec> specs = {
      {.folder = "ClassVariableLifetime",
       .check = CheckClassVariableLifetimeFixable},

      {.folder = "TimeValue",
       .check = [](const SL::FileContent* fC, SL::ErrorContainer* e,
                   SL::SymbolTable* s,
                   FixSourceManager& sm) -> std::vector<LintDiagnostic> {
         return CheckTimeValueFixable(fC, e, s, sm);
       }},

      {.folder = "WildcardEqualityOperator",
       .check = [](const SL::FileContent* fC, SL::ErrorContainer* e,
                   SL::SymbolTable* s,
                   FixSourceManager& sm) -> std::vector<LintDiagnostic> {
         return CheckWildcardEqualityOperatorFixable(fC, e, s, sm);
       }},

      {.folder = "WildcardInequalityOperator",
       .check = [](const SL::FileContent* fC, SL::ErrorContainer* e,
                   SL::SymbolTable* s,
                   FixSourceManager& sm) -> std::vector<LintDiagnostic> {
         return CheckWildcardInequalityOperatorFixable(fC, e, s, sm);
       }},

      {.folder = "TypeCasting", .check = CheckTypeCastingFixable},

      {.folder = "AssignmentPattern", .check = CheckAssignmentPatternFixable},

      {.folder = "ExponentFormatTimeValue",
       .check = [](const SL::FileContent* fC, SL::ErrorContainer* e,
                   SL::SymbolTable* s,
                   FixSourceManager& sm) -> std::vector<LintDiagnostic> {
         return CheckExponentFormatTimeValueFixable(fC, e, s, sm);
       }},

      {.folder = "MultipleDotStarConnections",
       .check = CheckMultipleDotStarConnectionsFixable},

      {.folder = "VoidCastOfVoidFunction",
       .check = CheckVoidCastOfVoidFunctionFixable},
  };
  return specs;
}

auto FixableGlobalSpecs() -> const std::vector<FixableGlobalRuleSpec>& {
  static const std::vector<FixableGlobalRuleSpec> specs = {
      {.folder = "MethodOverrideArgumentName",
       .check = CheckMethodOverrideArgumentNameFixable},
  };
  return specs;
}

class FixableRuleTest : public ::testing::TestWithParam<FixableRuleSpec> {};
class FixableGlobalRuleTest
    : public ::testing::TestWithParam<FixableGlobalRuleSpec> {};

void PrintTo(const FixableRuleSpec& spec, std::ostream* os) {
  *os << spec.folder;
}
void PrintTo(const FixableGlobalRuleSpec& spec, std::ostream* os) {
  *os << spec.folder;
}

}  // namespace

TEST_P(FixableRuleTest, FixProducesExpectedOutput) {
  RunFixableRuleTest(GetParam());
}

TEST_P(FixableGlobalRuleTest, FixProducesExpectedOutput) {
  RunFixableGlobalRuleTest(GetParam());
}

INSTANTIATE_TEST_SUITE_P(
    AllFixableRules, FixableRuleTest, ::testing::ValuesIn(FixableSpecs()),
    [](const ::testing::TestParamInfo<FixableRuleSpec>& info) {
      return info.param.folder;
    });

INSTANTIATE_TEST_SUITE_P(
    AllFixableGlobalRules, FixableGlobalRuleTest,
    ::testing::ValuesIn(FixableGlobalSpecs()),
    [](const ::testing::TestParamInfo<FixableGlobalRuleSpec>& info) {
      return info.param.folder;
    });
