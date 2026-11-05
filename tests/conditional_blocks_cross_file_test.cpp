#include "rules/conditional_blocks_cross_file.h"

#include <gtest/gtest.h>

#include <filesystem>
#include <unordered_set>

#include "main/lint_rules.h"
#include "utils.h"

namespace fs = std::filesystem;

namespace {

auto BasePath() -> const fs::path {
  return fs::current_path() / ".." / ".." / "tests" / "ConditionalChecks";
}

}  // namespace

TEST(ConditionalBlocksCrossFile, NoError) {
  const fs::path tests_path{BasePath() / "ConditionalBlocksCrossFile" /
                            "NoError"};

  global::CheckWithNoErrorsExpected(tests_path,
                                    CheckConditionalBlocksCrossFile);
}

TEST(ConditionalBlocksCrossFile, RaiseError) {
  const fs::path tests_path{BasePath() / "ConditionalBlocksCrossFile" /
                            "RaiseError"};

  const std::unordered_set<SURELOG::ErrorDefinition::ErrorType> ignoreList{};

  global::CheckWithErrorsExpected(
      tests_path, verihogg_lint::LINT_CONDITIONAL_BLOCKS_CROSS_FILE, ignoreList,
      CheckConditionalBlocksCrossFile);
}
