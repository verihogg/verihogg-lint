#include "rules/include_without_path.h"

#include <gtest/gtest.h>

#include <filesystem>
#include <unordered_set>

#include "main/lint_rules.h"
#include "utils.h"

namespace fs = std::filesystem;

namespace {

auto BasePath() -> const fs::path {
  return fs::current_path() / ".." / ".." / "tests" / "IncludeChecks";
}

}  // namespace

TEST(IncludeWithPathTest, NoError) {
  const fs::path tests_path{BasePath() / "IncludeWithPath" / "NoError"};

  global::CheckWithNoErrorsExpected(tests_path, CheckIncludeWithoutPath);
}

TEST(IncludeWithPathTest, RaiseError) {
  const fs::path tests_path{BasePath() / "IncludeWithPath" / "RaiseError"};

  const std::unordered_set<SURELOG::ErrorDefinition::ErrorType> ignoreList{};

  global::CheckWithErrorsExpected(tests_path,
                                  verihogg_lint::LINT_INCLUDE_WITH_PATH,
                                  ignoreList, CheckIncludeWithoutPath);
}
