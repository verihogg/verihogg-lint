#include <gtest/gtest.h>

#include <filesystem>

#include "main/lint_rules.h"
#include "rules/legal_file_extensions.h"
#include "utils.h"

namespace fs = std::filesystem;

namespace {

auto BasePath() -> const fs::path {
  return fs::current_path() / ".." / ".." / "tests" / "FileChecks";
}

}  // namespace

TEST(IllegalFileExtensionTest, NoError) {
  const fs::path tests_path{BasePath() / "IllegalFileExtension" / "NoError"};

  test::CheckWithNoErrorsExpected(tests_path, CheckLegalFileExtensions);
}

TEST(IllegalFileExtensionTest, RaiseError) {
  const fs::path tests_path{BasePath() / "IllegalFileExtension" / "RaiseError"};

  const std::unordered_set<SURELOG::ErrorDefinition::ErrorType> ignoreList{};

  test::CheckWithErrorsExpected(tests_path,
                                verihogg_lint::LINT_ILLEGAL_FILE_EXTENSION,
                                ignoreList, CheckLegalFileExtensions);
}